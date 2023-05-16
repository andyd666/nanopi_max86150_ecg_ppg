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
int start_recording(uint32_t samp_freq);


#endif /* INCLUDE_PERIPHERAL_H_ */
