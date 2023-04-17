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

#include <peripheral.h>
#include <filework.h>
#include <max86150_defs.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>


#define I2C0_SDA_PIN (8)
#define I2C0_SCL_PIN (9)
#define I2C0_INT_PIN (7)

static int max86150_fd;


int init_gpio() {
    int ret;

    ret = wiringPiSetup();
    if (ret != 0) {
        d_print("wiringPiSetup() failed\n");
        return ret;
    }
    max86150_fd = wiringPiI2CSetup(MAX86150_DEV_ID);

    if (!max86150_fd) {
        d_print("wiringPiI2CSetup(0) failed\n");
        return -1;
    }

    d_print("%s: max86150_fd = %d\n", __func__, max86150_fd);

    return wiringPiI2CReadReg8(max86150_fd, 0xff);
}
