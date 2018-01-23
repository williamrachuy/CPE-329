#include "msp.h"
#include "ac_mode.h"

void ac_mode_init() {
    min = 0xFFFF;
    max = 0;
    rms = 0;
    rms_samples = 0;
}


uint64_t isqrt64 (uint64_t n) // OR isqrt16 ( uint16 n ) OR  isqrt8 ( uint8 n ) - respectively [ OR overloaded as isqrt (uint?? n) in C++ ]
{
    uint64_t root, remainder, place;

    root = 0;
    remainder = n;
    place = 0x4000000000000000; // OR place = 0x4000; OR place = 0x40; - respectively

    while (place > remainder)
        place = place >> 2;
    while (place)
    {
        if (remainder >= root + place)
        {
            remainder = remainder - root - place;
            root = root + (place << 1);
        }
        root = root >> 1;
        place = place >> 2;
    }
    return root;
}
