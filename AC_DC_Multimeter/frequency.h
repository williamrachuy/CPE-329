#ifndef FREQUENCY_H_
#define FREQUENCY_H_

#include "msp.h"

int64_t freq_period;
int32_t frequency;

void frequency_timer_init();
void TA0_0_IRQHandler(void);
void TA0_N_IRQHandler(void);

#endif
