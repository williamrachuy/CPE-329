#ifndef CLOCK_H_
#define CLOCK_H_

#include "msp.h"

#define FREQ_15_MHz 1500000
#define FREQ_03_MHz 3000000
#define FREQ_06_MHz 6000000
#define FREQ_12_MHz 12000000
#define FREQ_24_MHz 24000000
#define FREQ_48_MHz 48000000

void source_DCO(int source);
int get_current_DCO();

#endif
