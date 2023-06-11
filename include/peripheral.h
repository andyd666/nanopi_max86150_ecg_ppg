/*
 * filename: peripheral.h
 */

#ifndef INCLUDE_PERIPHERAL_H_
#define INCLUDE_PERIPHERAL_H_

#include <stdint.h>
#include <max86150_defs.h>

#define I2C0_BAUD_RATE (210000)

#define TOTAL_SIGNALS       (5)
#define MAX_SIGNALS_ALLOWED (4)

int init_gpio();
void deinit_gpio();
int init_max86150(struct max86150_configuration *max86150);
int reset_device();
int start_recording(struct max86150_configuration *max86150);
int stop_recording();
int write_max86150_register(int reg, int data);
int read_max86150_register(int reg, uint8_t *data, int num);
int read_max86150_FIFO_multiple(int count, uint8_t *data);


#endif /* INCLUDE_PERIPHERAL_H_ */
