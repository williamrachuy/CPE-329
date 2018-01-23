#ifndef _ADC_14_H
#define _ADC_14_H

#include "msp.h"

uint32_t adc14_temp;

void adc14_init();
void adc14_sample();
uint32_t adc14_data_set(uint32_t data);
uint32_t adc14_data_get();
float adc14_scale_get();

#endif
