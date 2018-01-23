#include "led.h"

void led_init() {
    P2->DIR = 0x07;
    P2->OUT &= ~0x07;
}

void led_set(int value) {
    value &= 0x07;
    P2->OUT = (P2->OUT & ~0x07) | value;
}
