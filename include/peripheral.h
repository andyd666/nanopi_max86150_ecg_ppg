/*
 * filename: peripheral.h
 */

#ifndef INCLUDE_PERIPHERAL_H_
#define INCLUDE_PERIPHERAL_H_

#include <stdint.h>

#define I2C0_BAUD_RATE (210000)

#define TOTAL_SIGNALS       (5)
#define MAX_SIGNALS_ALLOWED (4)

typedef enum {
    none   = 0,
    ppg1   = 1,
    ppg2   = 2,
    pilot1 = 4,
    pilot2 = 8,
    ecg    = 16
} signals;

typedef enum {
    /* this enum correlates with
     * FIFO Data Control Register 1  and
     * FIFO Data Control Register 2  */
    NONE       = 0,
    PPG_LED1   = 1,
    PPG_LED2   = 2,
    PILOT_LED1 = 5,
    PILOT_LED2 = 6,
    ECG        = 9,
    RESERVED   = 15
} dcr_slot;

int init_gpio();
int init_max86150(uint16_t freq, uint16_t sig);
int reset_device();


#endif /* INCLUDE_PERIPHERAL_H_ */
