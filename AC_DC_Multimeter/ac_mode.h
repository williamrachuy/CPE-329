#ifndef AC_MODE_H_
#define AC_MODE_H_

uint16_t min;
uint16_t max;
uint64_t rms;
uint64_t average_rms;
uint64_t rms_samples;
void ac_mode_init();
uint64_t isqrt64 (uint64_t n);

#endif
