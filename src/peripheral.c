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
#include <filework.h>
#include <max86150_defs.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>


#define I2C0_WPI_SDA_PIN (8)
#define I2C0_WPI_SCL_PIN (9)
#define I2C0_WPI_INT_PIN (7)

static int max86150_fd;

static dcr_slot set_dcr_slot(uint16_t sig);
static int write_max86150_register(int reg, int data);
static int check_sampling_frequency(uint16_t freq, uint16_t sig);


int init_gpio() {
    int ret;

    ret = wiringPiSetup();
    if (ret != 0) {
        d_print("%s: wiringPiSetup() failed\n", __func__);
        return ret;
    }
    max86150_fd = wiringPiI2CSetup(MAX86150_DEV_ID);

    if (!max86150_fd) {
        d_print("%s: wiringPiI2CSetup(0) failed\n", __func__);
        return -1;
    }

    d_print("%s: max86150_fd = %d\n", __func__, max86150_fd);

    piLock(0);
    ret = wiringPiI2CReadReg8(max86150_fd, MAX86150_REG_PART_ID);
    if (MAX86150_PART_ID != ret) {
        d_print("%s: MAX86150 Part ID is %d. Must be 0x%02x. Device may be broken\n",
                __func__, ret, MAX86150_PART_ID);
        piUnlock(0);
        return -1;
    }
    piUnlock(0);

    return 0;
}

int init_max86150(uint16_t freq, uint16_t sig) {
    int i;
    int enabled_signals = 0;
    uint8_t reg_write_data;

    d_print("%s: freq = %u, sig = 0x%02x\n", __func__, freq, sig);

    if (!sig) {
        d_print("%s: no signal enabled\n", __func__);
        return -1;
    }

    if (sig == 0x1F) {
        d_print("%s: cannot allow every signal\n", __func__);
        return -1;
    }

    if (check_sampling_frequency(freq, sig)) {
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

    /* Form data for MAX86150_REG_SYS_CTL */
    piLock(0);
    if (write_max86150_register(MAX86150_REG_SYS_CTL, MAX86150_BIT_FIFO_EN)) {
        d_print("%s: cannot enable FIFO\n", __func__);
        piUnlock(0);
        return -1;
    }
    piUnlock(0);

    /* Form data for MAX86150_REG_FIFO_DCR1 */
    reg_write_data = 0;
    enabled_signals = 0;
    for (i = 0; i < TOTAL_SIGNALS; i++) {
        if ((1 << i) & sig) {
            enabled_signals++;
        }
        if (enabled_signals == 1) {
            reg_write_data |= set_dcr_slot((1 << i) & sig);
        } else if (enabled_signals == 2) {
            reg_write_data |= (set_dcr_slot((1 << i) & sig) << 4);
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
        if ((1 << i) & sig) {
            enabled_signals++;
        }
        if (enabled_signals == 3) {
            reg_write_data |= set_dcr_slot((1 << i) & sig);
        } else if (enabled_signals == 4) {
            reg_write_data |= (set_dcr_slot((1 << i) & sig) << 4);
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

static int check_sampling_frequency(uint16_t freq, uint16_t sig) {
    int i;
    int enabled_signals = 0;
    int max_bits_per_sec;

    if (!freq) {
        d_print("%s: sampling frequency not set\n", __func__);
    }

    for (i = 0; i < TOTAL_SIGNALS; i++) {
        if ((1 << i) & sig) {
            enabled_signals++;
        }
    }

    /* We need to make sure we can read data faster than it is being sampled.
     * We are limited by I2C speed. It is 210000 Baud/s. Even though it is
     * less than MAX86150 maximum of 400000 B/s, I couldn't reach higher
     * speed than 250000 B/s. But even on that speed MAX86150 become
     * unresponsive, so 210000 B/s is our practical limit. */

    max_bits_per_sec = enabled_signals * BITS_PER_FIFO_READ * freq;

    if (max_bits_per_sec > (I2C0_BAUD_RATE >> 1)) {
        d_print("%s: max Baudrate %d; asked for %d\n",
                __func__, I2C0_BAUD_RATE, (max_bits_per_sec << 1));
        return -1;
    }

    return 0;
}

static int write_max86150_register(int reg, int data) {
    d_print("%s: setting for fd=%d \treg 0x%02x \tdata 0x%02x\n", __func__, max86150_fd, reg, data);
    return wiringPiI2CWriteReg8(max86150_fd, reg, data);
}

int reset_device() {
    d_print("%s: resetting MAX86150\n", __func__);
    return wiringPiI2CWriteReg8(max86150_fd, MAX86150_REG_SYS_CTL, MAX86150_BIT_RESET);
}

