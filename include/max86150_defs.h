/*
 * filename: max86150_defs.h
 */

#ifndef __MAX86150_defs__
#define __MAX86150_defs__

#define BYTES_PER_FIFO_READ (3)
#define BITS_PER_FIFO_READ  (BYTES_PER_FIFO_READ << 3)

#define MAX86150_DEV_ID (0x5e)

/* Status Registers */
#define MAX86150_REG_IS1          (0x00) /* Interrupt Status 1           */
#define MAX86150_REG_IS2          (0x01) /* Interrupt Status 2           */
#define MAX86150_REG_IE1          (0x02) /* Interrupt Enable 1           */
#define MAX86150_REG_IE2          (0x03) /* Interrupt Enable 2           */

/* FIFO Registers */
#define MAX86150_REG_FIFO_WP      (0x04) /* FIFO Write Pointer           */
#define MAX86150_REG_FIFO_OVC     (0x05) /* Overflow Counter             */
#define MAX86150_REG_FIFO_RP      (0x06) /* FIFO Read Pointer            */
#define MAX86150_REG_FIFO_DR      (0x07) /* FIFO Data Register           */
#define MAX86150_REG_FIFO_CONF    (0x08) /* FIFO Configuration           */

/* FIFO Data Control */
#define MAX86150_REG_FIFO_DCR1    (0x09) /* FIFO Data Control Register 1 */
#define MAX86150_REG_FIFO_DCR2    (0x0A) /* FIFO Data Control Register 2 */

/* System Control */
#define MAX86150_REG_SYS_CTL      (0x0D) /* System Control               */

/* PPG Configuration */
#define MAX86150_REG_PPG_CFG1     (0x0E) /* PPG Configuration 1          */
#define MAX86150_REG_PPG_CFG2     (0x0F) /* PPG Configuration 2          */
#define MAX86150_REG_PROX_INT_TH  (0x10) /* Prox Interrupt Threshold     */

/* LED Pulse Amplitude */
#define MAX86150_REG_LED1_PA      (0x11) /* LED1 PA                      */
#define MAX86150_REG_LED2_PA      (0x12) /* LED2 PA                      */
#define MAX86150_REG_LED_RANGE    (0x14) /* LED Range                    */
#define MAX86150_REG_LED_PILOT_PA (0x15) /* LED1 PILOT PA                */

/* ECG Configuration */
#define MAX86150_REG_ECG_CFG1     (0x3C) /* ECG Configuration 1          */
#define MAX86150_REG_ECG_CFG3     (0x3E) /* ECG Configuration 3          */

/* Part ID */
#define MAX86150_REG_PART_ID      (0xFF) /* Part ID                      */

///////////////////////////////////////////////////////////////////////////

/* Interrupt Status 1           */
#define MAX86150_BIT_A_FULL             (1 << 7)
#define MAX86150_BIT_PPG_RDY            (1 << 6)
#define MAX86150_BIT_ALC_OVF            (1 << 5)
#define MAX86150_BIT_PROX_INT           (1 << 4)
#define MAX86150_BIT_PWR_RDY            (1 << 0)

/* Interrupt Status 2           */
#define MAX86150_BIT_VDD_OOR            (1 << 7)
#define MAX86150_BIT_ECG_RDY            (1 << 2)

/* Interrupt Enable 1           */
#define MAX86150_BIT_A_FULL_EN          (1 << 7)
#define MAX86150_BIT_PPG_RDY_EN         (1 << 6)
#define MAX86150_BIT_ACL_OVF_EN         (1 << 5)
#define MAX86150_BIT_PROX_INT_EN        (1 << 4)

/* Interrupt Enable 2           */
#define MAX86150_BIT_VDD_OOR_EN         (1 << 7)
#define MAX86150_BIT_ECG_RDY_EN         (1 << 2)

/* FIFO Write Pointer           */
#define MAX86150_BIT_FIFO_WP_PTR        (0x1F)

/* Overflow Counter             */
#define MAX86150_BIT_OVF_COUNTER        (0x1F)

/* FIFO Read Pointer            */
#define MAX86150_BIT_FIFO_RD_PTR        (0x1F)

/* FIFO Data Register           */
/* Full Register                */

/* FIFO Configuration           */
#define MAX86150_BIT_A_FULL_CLR         (1 << 6)
#define MAX86150_BIT_A_FULL_TYPE        (1 << 5)
#define MAX86150_BIT_FIFO_ROLLS_ON_FULL (1 << 4)
#define MAX86150_BIT_FIFO_A_FULL        (0x0F)

/* FIFO Data Control Register 1 */
#define MAX86150_BIT_FD2                (0x0F << 4)
#define MAX86150_BIT_FD1                (0x0F << 0)

/* FIFO Data Control Register 2 */
#define MAX86150_BIT_FD4                (0x0F << 4)
#define MAX86150_BIT_FD3                (0x0F << 0)

/* System Control               */
#define MAX86150_BIT_FIFO_EN            (1 << 2)
#define MAX86150_BIT_SHDN               (1 << 1)
#define MAX86150_BIT_RESET              (1 << 0)

/* PPG Configuration 1          */
#define MAX86150_BIT_PPG_ADC_RGE        (0x03 << 6)
#define MAX86150_BIT_PPG_SR             (0x0F << 2)
#define MAX86150_BIT_PPG_LED_PW         (0x03 << 0)

/* PPG Configuration 1          */
#define MAX86150_BIT_SMP_AVE            (0x07 << 0)

/* Prox Interrupt Threshold     */
/* Full Register                */

/* LED1 PA                      */
/* Full Register                */

/* LED2 PA                      */
/* Full Register                */

/* LED Range                    */
#define MAX86150_BIT_LED2_RGE           (0x03 << 2)
#define MAX86150_BIT_LED1_RGE           (0x03 << 0)

/* LED1 PILOT PA                */
/* Full Register                */

/* ECG Configuration 1          */
#define MAX86150_BIT_ECG_ADC_CLK        (1 << 2)
#define MAX86150_BIT_ECG_ADC_OSR        (0x03 << 0)

/* ECG Configuration 3          */
#define MAX86150_BIT_PGA_ECG_GAIN       (0x03 << 2)
#define MAX86150_BIT_IA_GAIN            (0x03 << 0)

/* Part ID                      */
/* Full Register                */
#define MAX86150_PART_ID                (0x1E)

#endif /* __MAX86150_defs__ */
