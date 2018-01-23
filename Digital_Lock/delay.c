#include "delay.h"

int delay_ms(int n, int source) {
    int i, j, limit;

    if (source == FREQ_48_MHz) {
        limit = LIMIT_48;
    }
    else if (source == FREQ_15_MHz) {
        limit = LIMIT_15;
    }
    else {
        limit = source / MAGIC;
    }

    for (j = 0; j < n; j++) {
        for (i = 0; i < limit; i++) {
        }
    }

    return 1;
}

/* uS delay only works with 48MHz */
int delay_us(int n, int source) {
    if (source == FREQ_48_MHz) {
        n = (n - 1)*6;
        while (n-- > 0);
        return 1;
    }
    return -1;
}
