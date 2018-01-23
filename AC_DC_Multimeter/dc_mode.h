#ifndef DC_MODE_H_
#define DC_MODE_H_

#include "msp.h"

uint64_t dc_sum_ms;
uint64_t dc_sum_s;
uint64_t dc_samples_ms;
uint64_t dc_samples_s;
uint64_t dc_average_ms;
uint64_t dc_average_s;

void dc_mode_init();

#endif
