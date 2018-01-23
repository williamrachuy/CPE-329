#include "msp.h"
#include "adc14.h"

struct adc14_settings {
    uint32_t data;
    uint8_t flag;
    float scale;
} adc14_config;


void adc14_init() {
    // GPIO Setup
    P5->SEL1 |= BIT4;                       // Configure P5.4 for ADC
    P5->SEL0 |= BIT4;

    // Sampling time, S&H=16, ADC14 on
    ADC14->CTL0 = ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP | ADC14_CTL0_ON;
    ADC14->CTL1 = ADC14_CTL1_RES_3;         // Use sampling timer, 14-bit conversion results

    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_1;   // A1 ADC input select; Vref=AVCC
    ADC14->IER0 |= ADC14_IER0_IE0;          // Enable ADC conv complete interrupt

    adc14_config.flag = 0;
    adc14_config.data = 0;
    adc14_config.scale = 0.0002032;

    // Enable ADC interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);
}

void adc14_sample() {
    ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
}

uint32_t adc14_data_set(uint32_t data) {
    return adc14_config.data = data;
}

uint32_t adc14_data_get() {
    return adc14_config.data;
}

float adc14_scale_get() {
    return adc14_config.scale;
}

