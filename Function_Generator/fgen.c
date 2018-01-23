#include "fgen.h"

// Look up tables for sine, square, sawtooth
int16_t lut_sine[TABLE_SIZE];
int16_t lut_square[TABLE_SIZE];
int16_t lut_sawtooth[TABLE_SIZE];
int16_t lut_fullcredit[TABLE_SIZE];

// Increment values for 100,200,300,400,500Hz with 2520 LUTS and ISR_PERIOD
int16_t inc_by_frequency[5] = {3,6,9,12,15};

struct state_settings {
    uint8_t wave;
    uint8_t frequency;
    float duty;
} fgen_state;

int16_t *lut;
volatile int lut_index = 0;
int square_upper;
int square_lower;


void init_fgen() {
    init_lut();
    change_duty_cycle(0.5);
    change_frequency(F100);
    change_wave(SQUARE);
}

void init_lut() {
    int i, j = 5;

    // Initializing the lookup table for sine, normalizing to table size
    lut_sine[0] = 0;
    for (i = 1; i < TABLE_SIZE; i++) {
        lut_sine[i] = (int16_t) ((TABLE_SIZE/2) * sin((i/(TABLE_SIZE*1.0)) * 2 * 3.14159 ));
    }

    // Normalizing sine table to 0->2048 for dac and 2520->4095
    for (i = 0; i < TABLE_SIZE; i++) {
        lut_sine[i] += TABLE_SIZE/2;
        lut_sine[i] = (int16_t)(lut_sine[i]*1.6253);
    }

    // Build the pwn sine
    lut_fullcredit[0] = 0;
    for (i = 1; i < TABLE_SIZE; i++) {
        lut_fullcredit[i] = (int16_t) ((TABLE_SIZE/2) * sin((i/(TABLE_SIZE*1.0)) * 2 * 3.14159 ));
    }

    for (i = 1; i < TABLE_SIZE; i+=2*j) {
        for (j = 0; j < 72; j++)
            lut_fullcredit[i + j] = -1*lut_fullcredit[i + j];
    }

    // Normalizing sine table to 0->2048 for dac and 2520->4095
    for (i = 0; i < TABLE_SIZE; i++) {
        lut_fullcredit[i] += TABLE_SIZE/2;
        lut_fullcredit[i] = (int16_t)(lut_fullcredit[i]*1.6253);
    }

    // Initialize lookup table for square
    for (i = 0; i < TABLE_SIZE/2; i++)
        lut_square[i] = DAC_LOW;
    for (; i < TABLE_SIZE; i++)
        lut_square[i] = DAC_HIGH;

    // Initialize the lookup table for sawtooth
    for (i = 0; i < TABLE_SIZE; i++)
        lut_sawtooth[i] = (int16_t)((i * DAC_HIGH)/TABLE_SIZE);

    return;
}

void change_wave(uint8_t wave) {
    switch (wave) {
    case SINE:
        waveform_isr = sawtooth_sine_isr;
        lut = lut_sine;
        fgen_state.wave = SINE;
        break;
    case SAWTOOTH:
        waveform_isr = sawtooth_sine_isr;
        lut = lut_sawtooth;
        fgen_state.wave = SAWTOOTH;
        break;
    case SQUARE:
        waveform_isr = square_isr;
        lut = lut_square;
        fgen_state.wave = SQUARE;
        break;
    case PWMSINE:
        waveform_isr = sawtooth_sine_isr;
        lut = lut_fullcredit;
        fgen_state.wave = PWMSINE;
    default:
        break;
    }
}

void change_duty_cycle(float new_duty) {
    // Compute the total steps based on current increment for 50 50 wave
    if (new_duty < -0.05 || new_duty > 1.05)
        return;
    if (fgen_state.wave == SQUARE) {
        square_lower = TABLE_SIZE*(1 - new_duty);
        square_upper = TABLE_SIZE;

        fgen_state.duty = new_duty;
    }
    return;
}

float get_duty_cycle() {
    return fgen_state.duty;
}

void change_frequency(uint8_t freq) {
    switch(freq) {
    case (F100):
        fgen_state.frequency = F100;
        break;
    case (F200):
        fgen_state.frequency = F200;
        break;
    case (F300):
        fgen_state.frequency = F300;
        break;
    case (F400):
        fgen_state.frequency = F400;
        break;
    case (F500):
        fgen_state.frequency = F500;
        break;
    default:
        break;
    }
}

uint16_t sawtooth_sine_isr() {
    int16_t drive_val;
    if (lut_index < TABLE_SIZE){
        drive_val = lut[lut_index];
        lut_index = lut_index + inc_by_frequency[fgen_state.frequency];
    } else {
        lut_index = 0;
        drive_val = lut[lut_index];
    }
    return drive_val;
}

uint16_t square_isr() {
    int16_t drive_val = 0;
    if (lut_index < square_lower) {
        drive_val = DAC_LOW;
        lut_index = lut_index + inc_by_frequency[fgen_state.frequency];
    } else if (lut_index < square_upper){
        drive_val = DAC_HIGH;
        lut_index = lut_index + inc_by_frequency[fgen_state.frequency];
    } else
        lut_index = 0;

    return drive_val;
}
