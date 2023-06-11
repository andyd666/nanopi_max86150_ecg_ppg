/**
 * filename: main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <max86150_defs.h>
#include <filework.h>
#include <peripheral.h>
#include <signalwork.h>

#define UNUSED(x) ((void)x)

static int validate_input(int argc, char **argv, struct max86150_configuration *max86150);
static void set_default_max86150_values(struct max86150_configuration *max86150);
static void print_usage(char **argv);


int main(int argc, char **argv) {
    int retval = 0;
    struct max86150_configuration max86150 = {0};
    uint8_t *read_buf = NULL;
    uint32_t *write_buf = NULL;
    ssize_t bytes_written;
    int write_buf_len_int;
    int binary_capture_file;
    int num_of_ppg; /* We need to know number of PPG signals to correctly pad with "0" elder bits */

    init_debug();

    set_default_max86150_values(&max86150);

    if (validate_input(argc, argv, &max86150)) {
        /* d_print("%s: cannot validate input\n", __func__); */ /* May be silently closed */
        retval = -1;
        goto cant_start;
    }

    if (init_gpio() == 0) {
        d_print("%s: init_gpio() successful\n", __func__);
    } else {
        d_print("%s: init_gpio() NOT successful\n", __func__);
        retval = -1;
        goto cant_start;
    }

    if (init_max86150(&max86150)) {
        retval = -1;
        goto cant_start;
    }

    read_buf = (uint8_t *)malloc(max86150.number_of_bytes_per_fifo_read * sizeof(typeof(read_buf[0])));
    if(!read_buf) {
        d_print("%s: cannot allocate memory for read_buf\n", __func__);
        retval = -1;
        goto cant_start;
    }

    num_of_ppg  = !!(max86150.allowed_signals & ppg1);
    num_of_ppg += !!(max86150.allowed_signals & ppg2);
    write_buf_len_int = max86150.number_of_bytes_per_fifo_read / 3;
    write_buf = (uint32_t *)malloc(write_buf_len_int * sizeof(typeof(write_buf[0])));
    if(!write_buf) {
        d_print("%s: cannot allocate memory for write_buf\n", __func__);
        retval = -1;
        goto cant_start;
    }

    binary_capture_file = open_capture_file(max86150.capture_file_name);
    if (-1 == binary_capture_file) {
        d_print("%s cannot open capture file \"%s\"\n",
                __func__, max86150.capture_file_name);
        retval = -1;
        goto cant_start;
    } else {
        uint32_t metadata = max86150.allowed_signals;

        metadata |= ((max86150.sampling_frequency & 0xffff) << 8);

        bytes_written = write(binary_capture_file, &metadata, sizeof(metadata));
        if (sizeof(metadata) != bytes_written) {
            d_print("%s: cannot write first byte of file, fd = %d\n", __func__, binary_capture_file);
            d_print("%s: errno = %d(%s)\n", __func__, errno, strerror(errno));
            retval = -1;
            goto cant_start;
        }
    }

    d_print("%s: read_buf_size %d\n", __func__, max86150.number_of_bytes_per_fifo_read * sizeof(typeof(read_buf[0])));

    if (register_term_signal()) {
        retval = -1;
        goto cant_start;
    }

    if (start_recording(&max86150)) {
        retval = 1;
        goto cant_start;
    }

    while (1) {
        uint8_t register_buffer[3];
        uint8_t read_pointer_val  = 0;
        uint8_t ovc_pointer_val   = 0;
        uint8_t write_pointer_val = 0;
        int i;
        int to_read_count;

        sleep(0xffffffff);
        if (get_sigint_status()) break;

        piLock(0);
        if(read_max86150_register(MAX86150_REG_FIFO_WP, register_buffer, 3))
        {
            piUnlock(0);
            d_print("%s: read FIFO WP/OVC/RP failed\n", __func__);
            break;
        }
        write_pointer_val = register_buffer[0];
        ovc_pointer_val   = register_buffer[1];
        read_pointer_val  = register_buffer[2];

        if (ovc_pointer_val) {
            piUnlock(0);
            d_print("%s: FIFO Overflow counter is not empty! Stopping recording\n", __func__);
            break;
        }

        to_read_count = (write_pointer_val > read_pointer_val) ?
                        (write_pointer_val - read_pointer_val) :
                        (32 + write_pointer_val - read_pointer_val);

        if (to_read_count < SAMPLES_PER_SINGLE_READ) {
            to_read_count = 0;
        } else if (to_read_count < (SAMPLES_PER_SINGLE_READ * 2)) {
            to_read_count = SAMPLES_PER_SINGLE_READ;
        } else if (to_read_count < (SAMPLES_PER_SINGLE_READ * 3)) {
            to_read_count = SAMPLES_PER_SINGLE_READ * 2;
        } else {
            to_read_count = SAMPLES_PER_SINGLE_READ * 3;
        }

        for (i = 0; i < to_read_count; i++) {
            int j;
            if (read_max86150_FIFO_multiple(max86150.number_of_bytes_per_fifo_read, read_buf)) {
                piUnlock(0);
                d_print("%s: FIFO read failed\n", __func__);
                break;
            }
            for (j = 0; j < write_buf_len_int; j++) {
                write_buf[j] = (read_buf[j * BYTES_PER_FIFO_READ + 2] << 0) |
                               (read_buf[j * BYTES_PER_FIFO_READ + 1] << 8) |
                               (read_buf[j * BYTES_PER_FIFO_READ + 0] << 16);
                if (j < num_of_ppg) {
                    write_buf[j] &= PPG_SIGNAL_MASK;
                }
            }
            bytes_written = write(binary_capture_file, write_buf, write_buf_len_int * sizeof(uint32_t));
            if ((write_buf_len_int * sizeof(uint32_t)) != (uint32_t)bytes_written) {
                d_print("%s: binary write failed, bytes written %d, fd = %d\n",
                        __func__, bytes_written, binary_capture_file);
                d_print("%s: errno = %d(%s)\n", __func__, errno, strerror(errno));
                retval = -1;
                break;
            }
        }
        if (i != to_read_count) break;
        piUnlock(0);
    }

    if (stop_recording()) {
        d_print("%s: cannot stop recording. Physical device reboot may be required\n", __func__);
        retval = -1;
        goto cant_start;
    }
    /* TODO: collect last data */

cant_start:
    if (read_buf) free(read_buf);
    deinit_gpio();
    close_capture_file();
    close_debug();
    return retval;
}


static int validate_input(int argc, char **argv, struct max86150_configuration *max86150) {
    int i;
    if (argc == 1) {
        print_usage(argv);
        return -1;
    }

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'h') {
                print_usage(argv);
                return -1;
            }
            if (argv[i][1] == 'f') {
                i++;
                max86150->sampling_frequency = atoi(argv[i]);
                if (!max86150->sampling_frequency) {
                    printf("%s: sample frequency is invalid - %s\n", __func__, argv[i]);
                    return -1;
                }
                continue;
            }
            if (0 == strcmp(argv[i], "--ppg1")) {
                max86150->allowed_signals |= ppg1;
                continue;
            }
            if (0 == strcmp(argv[i], "--ppg2")) {
                max86150->allowed_signals |= ppg2;
                continue;
            }
            if (0 == strcmp(argv[i], "--ppg")) {
                max86150->allowed_signals |= ppg1 | ppg2;
                continue;
            }
            if (0 == strcmp(argv[i], "--pilot1")) {
                max86150->allowed_signals |= pilot1;
                continue;
            }
            if (0 == strcmp(argv[i], "--pilot2")) {
                max86150->allowed_signals |= pilot2;
                continue;
            }
            if (0 == strcmp(argv[i], "--pilot")) {
                max86150->allowed_signals |= pilot1 | pilot2;
                continue;
            }
            if (0 == strcmp(argv[i], "--ecg")) {
                max86150->allowed_signals |= ecg;
                continue;
            }
            if (0 == strcmp(argv[i], "--set-ppg-range")) {
                max86150->ppg_adc_scale = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--set-led-pw")) {
                max86150->ppg_led_pw = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--set-ppg-pulses")) {
                max86150->ppg_pulses_reg = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--set-ppg-smp-ave")) {
                max86150->ppg_sample_average = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--set-led1-pulse-amplitude")) {
                max86150->ppg_led1_amplitude = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--set-led2-pulse-amplitude")) {
                max86150->ppg_led2_amplitude = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--set-led-pulse-amplitude")) {
                max86150->ppg_led1_amplitude = atoi(argv[++i]);
                max86150->ppg_led2_amplitude = max86150->ppg_led1_amplitude;
                continue;
            }
            if (0 == strcmp(argv[i], "--set-ecg-adc-clk")) {
                max86150->ecg_adc_clk_osr = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--set-ecg-pga-gain")) {
                max86150->ecg_pga_gain = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--set-ecg-ia-gain")) {
                max86150->ecg_ia_gain = atoi(argv[++i]);
                continue;
            }
            if (0 == strcmp(argv[i], "--capture_file_name")) {
                size_t size;

                i++;
                size = strlen(argv[i]);
                if (size >= MAX_FILENAME_LENGTH) {
                    d_print("%s file name too long %d\n", __func__, size);
                    return -1;
                }
                memcpy(max86150->capture_file_name, argv[i], size);
                max86150->capture_file_name[size] = 0;
                continue;
            }

            printf("%s, %d: unknown parameter - \"%s\"\n", __func__, __LINE__, argv[i]);
            print_usage(argv);
            return -1;
        } else {
            printf("%s, %d: unknown parameter - \"%s\"\n", __func__, __LINE__, argv[i]);
            print_usage(argv);
            return -1;
        }
    }

    return 0;
}

static void set_default_max86150_values(struct max86150_configuration *max86150) {
    max86150->capture_file_name[0]          = 0;
    max86150->number_of_bytes_per_fifo_read = 0;
    max86150->sampling_frequency            = 200;
    max86150->ppg_pulses_reg                = 1;
    max86150->ppg_adc_scale                 = 32;
    max86150->ppg_led_pw                    = 50;
    max86150->ppg_sample_average            = 1;
    max86150->ppg_led1_amplitude            = 25;
    max86150->ppg_led2_amplitude            = 25;
    max86150->ecg_adc_clk_osr               = 0;
    max86150->ecg_pga_gain                  = 2;
    max86150->ecg_ia_gain                   = 10;

    memcpy(max86150->capture_file_name, DEFAULT_BINARY_NAME, strlen(DEFAULT_BINARY_NAME));
    max86150->capture_file_name[strlen(DEFAULT_BINARY_NAME)] = 0;
}

static void print_usage(char **argv) {
    printf("%s usage:\n", argv[0]);
    printf("\t-h\t\t\t\t-\tshow usage\n");
    printf("\t---capture_file_name\t\t-\tSet output data file name\n\n");
    printf("\t--ppg1\t\t\t\t-\ttoggle on PPG1\n");
    printf("\t--ppg2\t\t\t\t-\ttoggle on PPG2\n");
    printf("\t--ppg\t\t\t\t-\ttoggle on both PPG signals\n");
    printf("\t--pilot1\t\t\t-\ttoggle on PILOT1\n");
    printf("\t--pilot2\t\t\t-\ttoggle on PILOT2\n");
    printf("\t--pilot\t\t\t\t-\ttoggle on both PILOT signals\n");
    printf("\tNote: at least one of the flags above must be enabled\n\n");
    printf("\t-f\t\t\t\t-\tset sampling frequency. Default 200 Hz\n");
    printf("\t--ecg\t\t\t\t-\ttoggle on ECG signal\n");
    printf("\t--set-ppg-range\t\t\t-\tPPG ADC range control [4ua(default), 8ua, 16ua, 32ua]\n");
    printf("\t--set-ppg-pulses\t\t-\tPPG pulses per sample [1(default), 2]\n");
    printf("\t--set-led-pw\t\t\t-\tset LED pulse width [50us(default), 100us, 200us, 400us]\n");
    printf("\t--set-ppg-smp-ave\t\t-\tPPG Sampling Average [1(default), 2, 4, 8, 16, 32]\n");
    printf("\t--set-led1-pulse-amplitude\t-\tset LED1 pulse amplitude current. Default 25mA\n");
    printf("\t--set-led2-pulse-amplitude\t-\tset LED2 pulse amplitude current. Default 25mA\n");
    printf("\t--set-led-pulse-amplitude\t-\tset both LEDs pulse amplitude current.\n");
    printf("\t\t\t\t\t\tIf range is  0 -  52 mA, then step is 1 mA\n");
    printf("\t\t\t\t\t\tIf range is 52 - 102 mA, then step is 2 mA\n");
    printf("\t--set-ecg-adc-clk\t\t-\tSet ECG ADC CLK [0(default), 1]\n");
    printf("\t--set-ecg-pga-gain\t\t-\tSet ECG PGA gain [1, 2(default), 4, 8]\n");
    printf("\t--set-ecg-ia-gain\t\t-\tSet ECG IA gain [5, 9/10(default), 20, 50]\n");
    printf("\t\t\t\t\t\tIA Gain 9/10 is 9.5. Both 9 or 10 can be used to set this value\n");
    printf("\tNote: \"-f200\" is invalid value. Please, separate flags and values\n");
}
