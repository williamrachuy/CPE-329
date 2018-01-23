#include "msp.h"
#include "dc_mode.h"

void dc_mode_init() {
    dc_sum_ms = 0;
    dc_samples_ms = 0;
    dc_samples_s = 0;
    dc_sum_s = 0;
}
