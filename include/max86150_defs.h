/*
 * filename: max86150_defs.h
 */

#ifndef __MAX86150_defs__
#define __MAX86150_defs__

#include <stdint.h>

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
#define MAX86150_SHIFT_PPG_ADC_RGE      (6)
#define MAX86150_SHIFT_PPG_SR           (2)
#define MAX86150_SHIFT_PPG_LED_PW       (0)
#define MAX86150_BIT_PPG_ADC_RGE        (0x03 << MAX86150_SHIFT_PPG_ADC_RGE)
#define MAX86150_BIT_PPG_SR             (0x0F << MAX86150_SHIFT_PPG_SR)
#define MAX86150_BIT_PPG_LED_PW         (0x03 << MAX86150_SHIFT_PPG_LED_PW)

/* PPG Configuration 1          */
#define MAX86150_BIT_SMP_AVE            (0x07 << 0)

/* Prox Interrupt Threshold     */
/* Full Register                */

/* LED1 PA                      */
/* Full Register                */

/* LED2 PA                      */
/* Full Register                */

/* LED Range                    */
#define MAX86150_SHIFT_LED2_RGE         (2)
#define MAX86150_SHIFT_LED1_RGE         (0)
#define MAX86150_BIT_LED2_RGE           (0x03 << MAX86150_SHIFT_LED2_RGE)
#define MAX86150_BIT_LED1_RGE           (0x03 << MAX86150_SHIFT_LED1_RGE)

/* LED1 PILOT PA                */
/* Full Register                */

/* ECG Configuration 1          */
#define MAX86150_BIT_ECG_ADC_CLK        (1 << 2)
#define MAX86150_BIT_ECG_ADC_OSR        (0x03 << 0)
#define MAX86150_MASK_ECG_ADC_CLK       (0x7)

/* ECG Configuration 3          */
#define MAX86150_MASK_PGA_IA_GAIN       (0x3)
#define MAX86150_SHIFT_PGA_GAIN         (2)
#define MAX86150_SHIFT_IA_GAIN          (0)
#define MAX86150_BIT_PGA_ECG_GAIN       (MAX86150_MASK_ECG_PGA_IA_GAIN << MAX86150_SHIFT_PGA_ECG_GAIN)
#define MAX86150_BIT_IA_GAIN            (MAX86150_MASK_ECG_PGA_IA_GAIN << MAX86150_SHIFT_IA_GAIN)


/* Part ID                      */
/* Full Register                */
#define MAX86150_PART_ID                (0x1E)


#define PPG_SIGNALS_ALLOW_EVERY_SIGNAL  (0x1F)
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

typedef enum {
    PPG_RGE_UA4  = 0,
    PPG_RGE_UA8  = 1,
    PPG_RGE_UA16 = 2,
    PPG_RGE_UA32 = 3
}ppg_adc_rge;

typedef enum {
    PPG_FREQ_INCORRECT = 0,
    PPG_FREQ_1_10   = 10,
    PPG_FREQ_1_20   = 20,
    PPG_FREQ_1_50   = 50,
    PPG_FREQ_1_84   = 84,
    PPG_FREQ_1_100  = 100,
    PPG_FREQ_1_200  = 200,
    PPG_FREQ_1_400  = 400,
    PPG_FREQ_1_800  = 800,
    PPG_FREQ_1_1000 = 1000,
    PPG_FREQ_1_1600 = 1600,
    PPG_FREQ_1_3200 = 3200,
    PPG_FREQ_2_10   = 10,
    PPG_FREQ_2_20   = 20,
    PPG_FREQ_2_50   = 50,
    PPG_FREQ_2_84   = 84,
    PPG_FREQ_2_100  = 100
}ppg_sample_per_second;

typedef enum {
    PPG_SR_INCORRECT = 0xff,
    PPG_SR_0000 = 0b0000,
    PPG_SR_0001 = 0b0001,
    PPG_SR_0010 = 0b0010,
    PPG_SR_0011 = 0b0011,
    PPG_SR_0100 = 0b0100,
    PPG_SR_0101 = 0b0101,
    PPG_SR_0110 = 0b0110,
    PPG_SR_0111 = 0b0111,
    PPG_SR_1000 = 0b1000,
    PPG_SR_1001 = 0b1001,
    PPG_SR_1010 = 0b1010,
    PPG_SR_1011 = 0b1011,
    PPG_SR_1100 = 0b1100,
    PPG_SR_1101 = 0b1101,
    PPG_SR_1110 = 0b1110,
    PPG_SR_1111 = 0b1111
}ppg_sample_code;

typedef enum {
    PPG_PULSE_WIDTH_50US  = 0x0,
    PPG_PULSE_WIDTH_100US = 0x1,
    PPG_PULSE_WIDTH_200US = 0x2,
    PPG_PULSE_WIDTH_400US = 0x3
}ppg_pulse_width;

typedef enum {
    PPG_PULSE_PER_SAMPLE_ONE = 1,
    PPG_PULSE_PER_SAMPLE_TWO = 2
}ppg_pulses_per_sample;

typedef enum {
    PPG_SMP_AVE_1 = 0,
    PPG_SMP_AVE_2 = 1,
    PPG_SMP_AVE_4 = 2,
    PPG_SMP_AVE_8 = 3,
    PPG_SMP_AVE_16 = 4,
    PPG_SMP_AVE_32 = 5
}ppg_smp_ave;

typedef uint8_t ppg_led_current_amplitude;

typedef enum {
    PPG_LED_CURRENT50  = 0x0,
    PPG_LED_CURRENT100 = 0x1
}ppg_led_current_range;

typedef enum {
    ECG_ADC_CLK_OSR_0_1600 = 0,
    ECG_ADC_CLK_OSR_0_800  = 1,
    ECG_ADC_CLK_OSR_0_400  = 2,
    ECG_ADC_CLK_OSR_0_200  = 3,
    ECG_ADC_CLK_OSR_1_3200 = 4,
    ECG_ADC_CLK_OSR_1_1600 = 5,
    ECG_ADC_CLK_OSR_1_800  = 6,
    ECG_ADC_CLK_OSR_1_400  = 7
}ecg_adc_clk_adc_osr_enum;

typedef enum {
    ECG_PGA_GAIN_1 = 0,
    ECG_PGA_GAIN_2 = 1,
    ECG_PGA_GAIN_4 = 2,
    ECG_PGA_GAIN_8 = 3
}ecg_pga_gain_enum;

typedef enum {
    ECG_IA_GAIN_5   = 0,
    ECG_IA_GAIN_9_5 = 1,
    ECG_IA_GAIN_20  = 2,
    ECG_IA_GAIN_50  = 3
}ecg_ia_gain_enum;

struct max86150_configuration {
    /* These parameters are entered by user */
    int                       sampling_frequency;
    uint8_t                   allowed_signals;

    int                       ppg_sampling_freq;
    int                       ecg_sampling_freq;
    int                       ppg_adc_scale;
    int                       ppg_led_pw;
    int                       ppg_sample_average;
    int                       ppg_led1_amplitude;
    int                       ppg_led2_amplitude;
    int                       ecg_adc_clk_osr;
    int                       ecg_pga_gain;
    int                       ecg_ia_gain;

    /* These values are writen into registers */
    ppg_adc_rge               ppg_range_reg;
    ppg_sample_code           ppg_sampling_reg;
    ppg_pulses_per_sample     ppg_pulses_reg;
    ppg_pulse_width           ppg_width_reg;
    ppg_smp_ave               ppg_smp_avg_reg;
    ppg_led_current_amplitude ppg_led1_amplitude_reg;
    ppg_led_current_amplitude ppg_led2_amplitude_reg;
    ppg_led_current_range     ppg_led1_amplitude_range;
    ppg_led_current_range     ppg_led2_amplitude_range;
    ecg_adc_clk_adc_osr_enum  ecg_adc_clk_osr_reg;
    ecg_pga_gain_enum         ecg_pga_gain_reg;
    ecg_ia_gain_enum          ecg_ia_gain_reg;
};

#endif /* __MAX86150_defs__ */
