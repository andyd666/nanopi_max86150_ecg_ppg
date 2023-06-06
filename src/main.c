/**
 * filename: main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <max86150_defs.h>
#include <filework.h>
#include <peripheral.h>

#define UNUSED(x) ((void)x)

static int validate_input(int argc, char **argv, struct max86150_configuration *max86150);
static void set_default_max86150_values(struct max86150_configuration *max86150);
static void print_usage(char **argv);


int main(int argc, char **argv) {
    int retval = 0;
    struct max86150_configuration max86150 = {0};

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

    if (start_recording(max86150.sampling_frequency)) {
        retval = 1;
        goto cant_start;
    }

    while (1) {

    }

cant_start:

    deinit_gpio();
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
    max86150->sampling_frequency = 200;
    max86150->ppg_pulses_reg     = 1;
    max86150->ppg_adc_scale      = 4;
    max86150->ppg_led_pw         = 50;
    max86150->ppg_sample_average = 1;
    max86150->ppg_led1_amplitude = 25;
    max86150->ppg_led2_amplitude = 25;
    max86150->ecg_adc_clk_osr    = 0;
}

static void print_usage(char **argv) {
    printf("%s usage:\n", argv[0]);
    printf("\t-h\t\t\t\t-\tshow usage\n\n");
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
    printf("\t--set-ecg-adc-clk\t\t-\tSet ECG ADC CLK [0(default), 1]");
    printf("\tNote: \"-f200\" is invalid value. Please, separate flags and values\n");
}
