/*
 * filename: peripheral.c
 *
 * This file contains all functions needed for peripheral setup and I2C related stuff
 */

/*
 * Nanopi air pins that are used for MAX86150 sensor
 * +-----+-----+-NanoPi-NEO-Air--+---+----------+
 * | BCM | wPi |   Name   | Mode | V | Physical |
 * +-----+-----+----------+------+---+----------+
 * |     |     |     3.3V |      |   |       1  |
 * |  12 |   8 |  GPIOA12 | ALT5 | 0 |       3  | - SDA
 * |  11 |   9 |  GPIOA11 | ALT5 | 0 |       5  | - SCL
 * | 203 |   7 |  GPIOG11 |  OFF | 0 |       7  | - INT
 * |     |     |       0v |      |   |       9  |
 */

#include <stdint.h>
#include <peripheral.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <filework.h>
#include <max86150_defs.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "../include/signalwork.h"

#define UNUSED(x) ((void)x)

#define I2C0_WPI_SDA_PIN (8)
#define I2C0_WPI_SCL_PIN (9)
#define I2C0_WPI_INT_PIN (7)

static int max86150_fd;

static dcr_slot set_dcr_slot(uint16_t sig);
static int init_i2c0_for_max86150();
static int write_max86150_register(int reg, int data);
static int read_max86150_register(int reg, uint8_t *data);
static int check_sampling_frequency(struct max86150_configuration *max86150);
static int ppg_check_pulses_per_sample(struct max86150_configuration *max86150);
static int ppg_convert_freq_to_register_value(struct max86150_configuration *max86150);
static int ppg_set_range(struct max86150_configuration *max86150);
static int ppg_set_led_pw(struct max86150_configuration *max86150);
static int ppg_set_smp_ave(struct max86150_configuration *max86150);
static int ppg_set_leds_range(struct max86150_configuration *max86150);
static int ecg_set_sampling_rate(struct max86150_configuration *max86150);
static int ecg_set_gains(struct max86150_configuration *max86150);


int init_gpio() {
    uint8_t reg_rd_buf[1] = {0};

    if (wiringPiSetup()) {
        d_print("%s: wiringPiSetup() failed\n", __func__);
        return -1;
    }

    if (init_i2c0_for_max86150()) {
        d_print("%s: init_i2c0_for_max86150() failed\n", __func__);
        return -1;
    }

    d_print("%s: max86150_fd = %d\n", __func__, max86150_fd);

    piLock(0);
    if (read_max86150_register(MAX86150_REG_PART_ID, reg_rd_buf)) {
        d_print("%s: read unsuccessful\n", __func__);
    }
    if (MAX86150_PART_ID != reg_rd_buf[0]) {
        d_print("%s: MAX86150 Part ID is 0x%02x. Must be 0x%02x\n",
                __func__, reg_rd_buf[0], MAX86150_PART_ID);
        piUnlock(0);
        return -1;
    }
    piUnlock(0);

    return 0;
}

void deinit_gpio() {
    if (max86150_fd) close(max86150_fd);
}

int init_max86150(struct max86150_configuration *max86150) {
    int i;
    int enabled_signals = 0;
    uint8_t reg_write_data;

    d_print("%s: freq = %u, sig = 0x%02x - ppg1(%c) ppg2(%c) pilot1(%c) pilot2(%c) ecg(%c)\n",
            __func__,
            max86150->sampling_frequency,
            max86150->allowed_signals,
            (max86150->allowed_signals & ppg1) ? 'T': 'F',
            (max86150->allowed_signals & ppg2) ? 'T': 'F',
            (max86150->allowed_signals & pilot1) ? 'T': 'F',
            (max86150->allowed_signals & pilot2) ? 'T': 'F',
            (max86150->allowed_signals & ecg) ? 'T': 'F');

    if (!max86150->allowed_signals) {
        d_print("%s: no signal enabled\n", __func__);
        return -1;
    }

    if (max86150->allowed_signals == PPG_SIGNALS_ALLOW_EVERY_SIGNAL) {
        d_print("%s: cannot allow every signal\n", __func__);
        return -1;
    }

    if (check_sampling_frequency(max86150)) {
        d_print("%s: wrong baudrate\n", __func__);
        return -1;
    }

    piLock(0);
    if (reset_device()) {
        d_print("%s: cannot reset device\n", __func__);
        piUnlock(0);
        return -1;
    }
    piUnlock(0);

    /* Form data for MAX86150_REG_PPG_CFG1 */
    reg_write_data = 0;
    if (max86150->allowed_signals & (ppg1 | ppg2)) {
        int tempret = 0;
        tempret += ppg_set_range(max86150);
        tempret += ppg_convert_freq_to_register_value(max86150);
        tempret += ppg_check_pulses_per_sample(max86150);
        tempret += ppg_set_led_pw(max86150);
        if (tempret) {
            d_print("%s: wrong data for MAX86150_REG_PPG_CFG1 register\n", __func__);
            return -1;
        }
        reg_write_data |= ((max86150->ppg_range_reg << MAX86150_SHIFT_PPG_ADC_RGE) & MAX86150_BIT_PPG_ADC_RGE);
        reg_write_data |= ((max86150->ppg_sampling_reg << MAX86150_SHIFT_PPG_SR) & MAX86150_BIT_PPG_SR);
        reg_write_data |= ((max86150->ppg_width_reg << MAX86150_SHIFT_PPG_LED_PW) & MAX86150_BIT_PPG_LED_PW);
    }
    piLock(0);
    if (write_max86150_register(MAX86150_REG_PPG_CFG1, reg_write_data)) {
        piUnlock(0);
        d_print("%s: write unsuccessful\n", __func__);
        return -1;
    }
    piUnlock(0);

    /* Form data for MAX86150_REG_PPG_CFG2 */
    reg_write_data = 0;
    if (max86150->allowed_signals & (ppg1 | ppg2)) {
        if (ppg_set_smp_ave(max86150)) {
            d_print("%s: wrong data for MAX86150_REG_PPG_CFG2 register\n", __func__);
            return -1;
        }
        reg_write_data = max86150->ppg_smp_avg_reg & MAX86150_BIT_SMP_AVE;
    }
    piLock(0);
    if (write_max86150_register(MAX86150_REG_PPG_CFG2, reg_write_data)) {
        piUnlock(0);
        d_print("%s: write unsuccessful\n", __func__);
        return -1;
    }
    piUnlock(0);

    /* Form data for MAX86150_REG_LED1_PA */
    /* Form data for MAX86150_REG_LED2_PA */
    /* Form data for MAX86150_REG_LED_RANGE */
    reg_write_data = 0;
    if (max86150->allowed_signals & (ppg1 | ppg2)) {
        if (ppg_set_leds_range(max86150)) {
            d_print("%s: cannot set LED current range - %d/%d\n",
                    __func__, max86150->ppg_led1_amplitude, max86150->ppg_led2_amplitude);
            return -1;
        }
        reg_write_data |= ((max86150->ppg_led1_amplitude_range << MAX86150_SHIFT_LED1_RGE) & MAX86150_BIT_LED1_RGE);
        piLock(0);
        if (write_max86150_register(MAX86150_REG_LED1_PA, max86150->ppg_led1_amplitude_reg)) {
            piUnlock(0);
            d_print("%s: write unsuccessful\n", __func__);
            return -1;
        }
        reg_write_data |= ((max86150->ppg_led2_amplitude_range << MAX86150_SHIFT_LED2_RGE) & MAX86150_BIT_LED2_RGE);
        if (write_max86150_register(MAX86150_REG_LED2_PA, max86150->ppg_led2_amplitude_reg)) {
            piUnlock(0);
            d_print("%s: write unsuccessful\n", __func__);
            return -1;
        }
        if (write_max86150_register(MAX86150_REG_LED_RANGE, reg_write_data)) {
            piUnlock(0);
            d_print("%s: write unsuccessful\n", __func__);
            return -1;
        }
        piUnlock(0);
    }

    /* Form data for MAX86150_REG_ECG_CFG1 */
    reg_write_data = 0;
    if (max86150->allowed_signals & ecg) {
        if (ecg_set_sampling_rate(max86150)) {
            d_print("%s: incorrect sampling rate - %d\n", __func__, max86150->sampling_frequency);
            return -1;
        }
        reg_write_data |= (max86150->ecg_adc_clk_osr_reg) & MAX86150_MASK_ECG_ADC_CLK;
    }
    piLock(0);
    if (write_max86150_register(MAX86150_REG_ECG_CFG1, reg_write_data)) {
        piUnlock(0);
        d_print("%s: write unsuccessful\n", __func__);
        return -1;
    }
    piUnlock(0);

    /* Form data for MAX86150_REG_ECG_CFG3 */
    reg_write_data = 0;
    if (max86150->allowed_signals & ecg) {
        if (ecg_set_gains(max86150)) {
            d_print("%s: incorrect gain - PGA %d; IA %d\n",
                    __func__, max86150->ecg_pga_gain, max86150->ecg_ia_gain);
            return -1;
        }
        reg_write_data |= (((max86150->ecg_pga_gain_reg) & MAX86150_MASK_PGA_IA_GAIN) << MAX86150_SHIFT_PGA_GAIN);
        reg_write_data |= (((max86150->ecg_ia_gain_reg) & MAX86150_MASK_PGA_IA_GAIN) << MAX86150_SHIFT_IA_GAIN);
    }
    piLock(0);
    if (write_max86150_register(MAX86150_REG_ECG_CFG3, reg_write_data)) {
        piUnlock(0);
        d_print("%s: write unsuccessful\n", __func__);
        return -1;
    }
    piUnlock(0);

    /* Form data for MAX86150_REG_FIFO_DCR1 */
    reg_write_data = 0;
    enabled_signals = 0;
    for (i = 0; i < TOTAL_SIGNALS; i++) {
        if ((1 << i) & max86150->allowed_signals) {
            enabled_signals++;
        }
        if (enabled_signals == 1) {
            reg_write_data |= set_dcr_slot((1 << i) & max86150->allowed_signals);
        } else if (enabled_signals == 2) {
            reg_write_data |= (set_dcr_slot((1 << i) & max86150->allowed_signals) << 4);
            break;
        }
    }
    piLock(0);
    if (write_max86150_register(MAX86150_REG_FIFO_DCR1, reg_write_data)) {
        piUnlock(0);
        d_print("%s: write unsuccessful\n", __func__);
        return -1;
    }
    piUnlock(0);

    /* Form data for MAX86150_REG_FIFO_DCR2 */
    reg_write_data = 0;
    for (++i; i < TOTAL_SIGNALS; i++) {
        if ((1 << i) & max86150->allowed_signals) {
            enabled_signals++;
        }
        if (enabled_signals == 3) {
            reg_write_data |= set_dcr_slot((1 << i) & max86150->allowed_signals);
        } else if (enabled_signals == 4) {
            reg_write_data |= (set_dcr_slot((1 << i) & max86150->allowed_signals) << 4);
            break;
        }
    }
    piLock(0);
    if (write_max86150_register(MAX86150_REG_FIFO_DCR2, reg_write_data)) {
        piUnlock(0);
        d_print("%s: write unsuccessful\n", __func__);
        return -1;
    }
    piUnlock(0);

    /* Form data for MAX86150_REG_SYS_CTL */
    piLock(0);
    if (write_max86150_register(MAX86150_REG_SYS_CTL, MAX86150_BIT_FIFO_EN)) {
        d_print("%s: cannot enable FIFO\n", __func__);
        piUnlock(0);
        return -1;
    }
    piUnlock(0);

    return 0;
}


int start_recording(struct max86150_configuration *max86150) {
    piLock(0);

    piUnlock(0);
    start_max86150_timer(max86150->sampling_frequency);
    return 0;
}

int stop_recording() {
    piLock(0);
    if(write_max86150_register(MAX86150_REG_SYS_CTL, 0)) {
        piUnlock(0);
        d_print("%s: cannot stop capturing\n", __func__);
    }
    if(write_max86150_register(MAX86150_REG_FIFO_DCR1, 0)) {
        piUnlock(0);
        d_print("%s: cannot stop capturing\n", __func__);
    }
    if(write_max86150_register(MAX86150_REG_FIFO_DCR1, 0)) {
        piUnlock(0);
        d_print("%s: cannot stop capturing\n", __func__);
    }
    piUnlock(0);

    if (stop_max86150_timer()) {
        d_print("%s: cannot stop timer\n");
        return -1;
    }

    return 0;
}


static dcr_slot set_dcr_slot(uint16_t sig) {
    switch (sig) {
    case none:
        return NONE;
    case ppg1:
        return PPG_LED1;
    case ppg2:
        return PPG_LED2;
    case pilot1:
        return PILOT_LED1;
    case pilot2:
        return PILOT_LED2;
    case ecg:
        return ECG;
    default:
        return RESERVED;
    }
}

static int init_i2c0_for_max86150() {
    if ((max86150_fd = open("/dev/i2c-0", O_RDWR)) < 0) {
        d_print("%s: Failed to open i2c bus /dev/i2c-0\n", __func__);
        return -1;
    }

    if (ioctl(max86150_fd, I2C_SLAVE, MAX86150_DEV_ID)) {
        d_print("%s: ioctl(%d, 0x%02x, 0x%02x) failed\n",
                __func__, max86150_fd, I2C_SLAVE, MAX86150_DEV_ID);
        return -1;
    }
    return 0;
}

static int check_sampling_frequency(struct max86150_configuration *max86150) {
    int i;
    int enabled_signals = 0;
    int max_bits_per_sec;

    if (!max86150->sampling_frequency) {
        d_print("%s: sampling frequency not set\n", __func__);
    }

    for (i = 0; i < TOTAL_SIGNALS; i++) {
        if ((1 << i) & max86150->allowed_signals) {
            enabled_signals++;
        }
    }

    /* We need to make sure we can read data faster than it is being sampled.
     * We are limited by I2C speed. It is 210000 Baud/s. Even though it is
     * less than MAX86150 maximum of 400000 B/s, I couldn't reach higher
     * speed than 250000 B/s. But even on that speed MAX86150 become
     * unresponsive, so 210000 B/s is our practical limit. */

    max_bits_per_sec = enabled_signals * BITS_PER_FIFO_READ * max86150->sampling_frequency;

    if (max_bits_per_sec > (I2C0_BAUD_RATE >> 1)) {
        d_print("%s: max Baudrate %d; asked for %d\n",
                __func__, I2C0_BAUD_RATE, (max_bits_per_sec << 1));
        return -1;
    }

    /* Setting PPG sampling frequency */
    if (max86150->allowed_signals & (ppg1 | ppg2)) {
        switch (max86150->sampling_frequency) {
            case PPG_FREQ_1_10:
            case PPG_FREQ_1_20:
            case PPG_FREQ_1_50:
            case PPG_FREQ_1_84:
            case PPG_FREQ_1_100:
            case PPG_FREQ_1_200:
            case PPG_FREQ_1_400:
            case PPG_FREQ_1_800:
            case PPG_FREQ_1_1000:
            case PPG_FREQ_1_1600:
            case PPG_FREQ_1_3200:
           /* case PPG_FREQ_2_10:  */
           /* case PPG_FREQ_2_20:  */
           /* case PPG_FREQ_2_50:  */
           /* case PPG_FREQ_2_84:  */
           /* case PPG_FREQ_2_100: */
                max86150->ppg_sampling_freq = max86150->sampling_frequency;
                break;
            default:
                d_print("%s: incorrect PPG sampling frequency - %d\n",
                        __func__, max86150->sampling_frequency);
                return -1;
        }
    }

    return 0;
}

static int ppg_check_pulses_per_sample(struct max86150_configuration *max86150) {

    if (max86150->ppg_pulses_reg == 1) return 0;
    if (max86150->ppg_pulses_reg == 2) {
        switch (max86150->ppg_sampling_reg) {
            case PPG_SR_0000:
                max86150->ppg_sampling_reg = PPG_SR_1011;
                return 0;
            case PPG_SR_0001:
                max86150->ppg_sampling_reg = PPG_SR_1100;
                return 0;
            case PPG_SR_0010:
                max86150->ppg_sampling_reg = PPG_SR_1101;
                return 0;
            case PPG_SR_0011:
                max86150->ppg_sampling_reg = PPG_SR_1110;
                return 0;
            case PPG_SR_0100:
                max86150->ppg_sampling_reg = PPG_SR_1111;
                return 0;
            default:
                d_print("%s: pulse width check failed - ppg_pulses = %d, ppg_sampling_reg = 0x%02x\n",
                        __func__, max86150->ppg_pulses_reg, max86150->ppg_sampling_reg);
                return -1;
        }
    }
    d_print("%s: pulse width check failed - ppg_pulses = %d, ppg_sampling = 0x%02x\n",
            __func__, max86150->ppg_pulses_reg, max86150->ppg_sampling_reg);
    return -1;
}

static int ppg_convert_freq_to_register_value(struct max86150_configuration *max86150) {

    switch(max86150->ppg_sampling_freq) {
    case PPG_FREQ_1_10:
   /* case PPG_FREQ_2_10:  */
        max86150->ppg_sampling_reg = PPG_SR_0000;
        return 0;
    case PPG_FREQ_1_20:
   /* case PPG_FREQ_2_20:  */
        max86150->ppg_sampling_reg = PPG_SR_0001;
        return 0;
    case PPG_FREQ_1_50:
   /* case PPG_FREQ_2_50:  */
        max86150->ppg_sampling_reg = PPG_SR_0010;
        return 0;
    case PPG_FREQ_1_84:
   /* case PPG_FREQ_2_84:  */
        max86150->ppg_sampling_reg = PPG_SR_0011;
        return 0;
    case PPG_FREQ_1_100:
   /* case PPG_FREQ_2_100: */
        max86150->ppg_sampling_reg = PPG_SR_0100;
        return 0;
    case PPG_FREQ_1_200:
        max86150->ppg_sampling_reg = PPG_SR_0101;
        return 0;
    case PPG_FREQ_1_400:
        max86150->ppg_sampling_reg = PPG_SR_0110;
        return 0;
    case PPG_FREQ_1_800:
        max86150->ppg_sampling_reg = PPG_SR_0111;
        return 0;
    case PPG_FREQ_1_1000:
        max86150->ppg_sampling_reg = PPG_SR_1000;
        return 0;
    case PPG_FREQ_1_1600:
        max86150->ppg_sampling_reg = PPG_SR_1001;
        return 0;
    case PPG_FREQ_1_3200:
        max86150->ppg_sampling_reg = PPG_SR_1010;
        return 0;
    default:
        d_print("%s: PPG sampling conversion failed - ppg_sampling_freq = %d\n",
                __func__, max86150->ppg_sampling_freq);
        return -1;
    }
}

static int ppg_set_range(struct max86150_configuration *max86150) {
    switch (max86150->ppg_adc_scale) {
        case 4:
            max86150->ppg_range_reg = PPG_RGE_UA4;
            return 0;
        case 8:
            max86150->ppg_range_reg = PPG_RGE_UA8;
            return 0;
        case 16:
            max86150->ppg_range_reg = PPG_RGE_UA16;
            return 0;
        case 32:
            max86150->ppg_range_reg = PPG_RGE_UA32;
            return 0;
        default:
            d_print("%s: PPG range incorrect - ppg_adc_scale = %d\n",
                    __func__, max86150->ppg_adc_scale);
            return -1;
    }
}

static int ppg_set_led_pw(struct max86150_configuration *max86150) {
    switch (max86150->ppg_led_pw) {
        case 50:
            max86150->ppg_width_reg = PPG_PULSE_WIDTH_50US;
            return 0;
        case 100:
            max86150->ppg_width_reg = PPG_PULSE_WIDTH_100US;
            return 0;
        case 200:
            max86150->ppg_width_reg = PPG_PULSE_WIDTH_200US;
            return 0;
        case 400:
            max86150->ppg_width_reg = PPG_PULSE_WIDTH_400US;
            return 0;
    }
    d_print("%s: LED pulse width incorrect - ppg_led_pw = %d\n",
            __func__, max86150->ppg_led_pw);
    return -1;
}

static int ppg_set_smp_ave(struct max86150_configuration *max86150) {
    switch (max86150->ppg_sample_average) {
        case 1:
            max86150->ppg_smp_avg_reg = PPG_SMP_AVE_1;
            return 0;
        case 2:
            max86150->ppg_smp_avg_reg = PPG_SMP_AVE_2;
            return 0;
        case 4:
            max86150->ppg_smp_avg_reg = PPG_SMP_AVE_4;
            return 0;
        case 8:
            max86150->ppg_smp_avg_reg = PPG_SMP_AVE_8;
            return 0;
        case 16:
            max86150->ppg_smp_avg_reg = PPG_SMP_AVE_16;
            return 0;
        case 32:
            max86150->ppg_smp_avg_reg = PPG_SMP_AVE_32;
            return 0;
    }
    d_print("%s: PPG sample average incorrect - %d\n",
            __func__, max86150->ppg_sample_average);
    return -1;
}

#define LED_AMPLITUDE_MULTIPLIER 5
static int ppg_set_leds_range(struct max86150_configuration *max86150) {
    if ((max86150->ppg_led1_amplitude > 0) && (max86150->ppg_led1_amplitude <=51)) {
        max86150->ppg_led1_amplitude_reg = max86150->ppg_led1_amplitude * LED_AMPLITUDE_MULTIPLIER;
        max86150->ppg_led1_amplitude_range = PPG_LED_CURRENT50;
    } else if ((max86150->ppg_led1_amplitude <= 102) && (max86150->ppg_led1_amplitude > 51)) {
        max86150->ppg_led1_amplitude_reg = (max86150->ppg_led1_amplitude >> 1) * LED_AMPLITUDE_MULTIPLIER;
        max86150->ppg_led1_amplitude_range = PPG_LED_CURRENT100;
    } else {
        d_print("%s: cannot set LED1 current amplitude - %d\n", __func__, max86150->ppg_led1_amplitude);
        return -1;
    }

    if ((max86150->ppg_led2_amplitude > 0) && (max86150->ppg_led2_amplitude <=51)) {
        max86150->ppg_led2_amplitude_reg = max86150->ppg_led2_amplitude * LED_AMPLITUDE_MULTIPLIER;
        max86150->ppg_led2_amplitude_range = PPG_LED_CURRENT50;
    } else if ((max86150->ppg_led2_amplitude <= 102) && (max86150->ppg_led2_amplitude > 51)) {
        max86150->ppg_led2_amplitude_reg = (max86150->ppg_led2_amplitude >> 1) * LED_AMPLITUDE_MULTIPLIER;
        max86150->ppg_led2_amplitude_range = PPG_LED_CURRENT100;
    } else {
        d_print("%s: cannot set LED2 current amplitude - %d\n", __func__, max86150->ppg_led2_amplitude);
        return -1;
    }

    return 0;
}

static int ecg_set_sampling_rate(struct max86150_configuration *max86150) {
    int is_ecg_edc_clk_enabled = 0;

    if (max86150->sampling_frequency == 3200 || max86150->ecg_adc_clk_osr) {
        is_ecg_edc_clk_enabled = 1;
    }

    if (!is_ecg_edc_clk_enabled) {
        switch (max86150->sampling_frequency) {
            case 1600:
                max86150->ecg_adc_clk_osr_reg = ECG_ADC_CLK_OSR_0_1600;
                break;
            case 800:
                max86150->ecg_adc_clk_osr_reg = ECG_ADC_CLK_OSR_0_800;
                break;
            case 400:
                max86150->ecg_adc_clk_osr_reg = ECG_ADC_CLK_OSR_0_400;
                break;
            case 200:
                max86150->ecg_adc_clk_osr_reg = ECG_ADC_CLK_OSR_0_200;
                break;
            default:
                d_print("%s: cannot set ECG sampling rate - %d\n", __func__, max86150->sampling_frequency);
                return -1;
        }
    } else {
        switch (max86150->sampling_frequency) {
            case 3200:
                max86150->ecg_adc_clk_osr_reg = ECG_ADC_CLK_OSR_1_3200;
                break;
            case 1600:
                max86150->ecg_adc_clk_osr_reg = ECG_ADC_CLK_OSR_1_1600;
                break;
            case 800:
                max86150->ecg_adc_clk_osr_reg = ECG_ADC_CLK_OSR_1_800;
                break;
            case 400:
                max86150->ecg_adc_clk_osr_reg = ECG_ADC_CLK_OSR_1_400;
                break;
            default:
                d_print("%s: cannot set ECG sampling rate - %d\n", __func__, max86150->sampling_frequency);
                return -1;
        }
    }
    max86150->ecg_sampling_freq = max86150->sampling_frequency;

    return 0;
}

static int ecg_set_gains(struct max86150_configuration *max86150) {
    switch (max86150->ecg_pga_gain) {
        case 1:
            max86150->ecg_pga_gain_reg = ECG_PGA_GAIN_1;
            break;
        case 2:
            max86150->ecg_pga_gain_reg = ECG_PGA_GAIN_2;
            break;
        case 4:
            max86150->ecg_pga_gain_reg = ECG_PGA_GAIN_4;
            break;
        case 8:
            max86150->ecg_pga_gain_reg = ECG_PGA_GAIN_8;
            break;
        default:
            d_print("%s: incorrect ECG PGA gain %d\n", __func__, max86150->ecg_pga_gain);
            return -1;
    }

    switch (max86150->ecg_ia_gain) {
        case 5:
            max86150->ecg_ia_gain_reg = ECG_IA_GAIN_5;
            break;
        case 9:
        case 10:
            max86150->ecg_ia_gain_reg = ECG_IA_GAIN_9_5;
            break;
        case 20:
            max86150->ecg_ia_gain_reg = ECG_IA_GAIN_20;
            break;
        case 50:
            max86150->ecg_ia_gain_reg = ECG_IA_GAIN_50;
            break;
        default:
            d_print("%s: incorrect ECG IA gain %d\n", __func__, max86150->ecg_ia_gain);
            return -1;
    }

    return 0;
}

static int write_max86150_register(int reg, int data) {
    int wr_bytes = 0;
    char buf[2];

    /* WRITE can work like this, while READ cannot for some reason */
    buf[0] = reg;
    buf[1] = data;
    d_print("%s: setting for fd=%d \treg 0x%02x \tdata 0x%02x - ", __func__, max86150_fd, reg, data);
    wr_bytes = write(max86150_fd, buf, 2);
    d_print("wr_bytes = %d\n", wr_bytes);
    return (wr_bytes == 2) ? 0 : wr_bytes;
}

static int read_max86150_register(int reg, uint8_t *data) {
    uint8_t outbuf[1];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];

    msgs[0].addr = MAX86150_DEV_ID;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = outbuf;

    msgs[1].addr = MAX86150_DEV_ID;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = 1;
    msgs[1].buf = data;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 2;

    outbuf[0] = reg;

    *(msgs[1].buf) = 0;
    if (ioctl(max86150_fd, I2C_RDWR, &msgset) < 0) {
        d_print("%s: ioctl(I2C_RDWR) in i2c_read\n", __func__);
        return -1;
    }

    return 0;
}

int reset_device() {
    int wr_bytes = 0;
    char buf[2];
    buf[0] = MAX86150_REG_SYS_CTL;
    buf[1] = MAX86150_BIT_RESET;
    d_print("%s: resetting MAX86150 - ", __func__);
    wr_bytes = write(max86150_fd, buf, 2);
    d_print("wr_bytes = %d\n", wr_bytes);
    return (wr_bytes == 2) ? 0 : wr_bytes;
}

