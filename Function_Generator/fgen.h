
#ifndef FGEN_H_
#define FGEN_H_

#include "msp.h"
#include <math.h>

#define TABLE_SIZE 2520
#define DAC_HIGH 4095
#define DAC_LOW 0

#define SQUARE 0
#define SINE 1
#define SAWTOOTH 2
#define PWMSINE 3

#define F100 0
#define F200 1
#define F300 2
#define F400 3
#define F500 4




uint16_t (*waveform_isr)(void);

// Begin function declarations
void init_fgen();
void init_lut();
void change_duty_cycle(float duty);
void change_frequency(uint8_t freq);
void change_wave(uint8_t wave);
float get_duty_cycle();
uint16_t sawtooth_sine_isr();
uint16_t square_isr();

#endif /* FGEN_H_ */
