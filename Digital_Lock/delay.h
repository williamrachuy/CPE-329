#ifndef DELAY_H_
#define DELAY_H_

#include "msp.h"
#include "clock.h"

#define MAGIC 10910         /* Ratio of frequency to cycles in order to achieve tuned 1ms delay, found experimentally*/
#define LIMIT_48 4050
#define LIMIT_15 136

int delay_ms(int n, int source);
int delay_us(int n, int source);

#endif
