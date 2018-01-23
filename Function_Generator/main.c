#include "msp.h"
#include "dac.h"
#include "clock.h"
#include "fgen.h"
#include "keypad.h"


#define HIGH 2520
#define LOW 0
#define ISR_PERIOD 287      //minimum 260

volatile char key = 0;
volatile char key_follower = 0;

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    source_DCO(FREQ_24_MHz);

    init_dac(P2, BIT5, EUSCI_B0);
    init_fgen();
    keypad_init();

    P1->SEL0 &= ~(BIT0);
    P1->SEL1 &= ~(BIT0);
    P1->DIR |= BIT0;
    P1->OUT |= BIT0;

    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
    TIMER_A0->CCR[0] = ISR_PERIOD; /*timer_settings.duty_lower*/;
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK |
            TIMER_A_CTL_MC__CONTINUOUS;    //0x00C0 | // SMCLK, continuous mode

    __enable_irq();
    NVIC->ISER[0] |= 1 << ((TA0_0_IRQn) & 31);

    while(1) {
        key_follower = key;

        key = keypad_getkey();

        if (key != key_follower) {
            if      (key_follower == '1') change_frequency(F100);
            else if (key_follower == '2') change_frequency(F200);
            else if (key_follower == '3') change_frequency(F300);
            else if (key_follower == '4') change_frequency(F400);
            else if (key_follower == '5') change_frequency(F500);
            else if (key_follower == '6') change_wave(PWMSINE);
            else if (key_follower == '7') change_wave(SQUARE);
            else if (key_follower == '8') change_wave(SAWTOOTH);
            else if (key_follower == '9') change_wave(SINE);
            else if (key_follower == '0') change_duty_cycle(0.5);
            else if (key_follower == '*') change_duty_cycle(get_duty_cycle() - 0.1);
            else if (key_follower == '#') change_duty_cycle(get_duty_cycle() + 0.1);
        }
    }

}

void TA0_0_IRQHandler(void) {
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    drive_dac(waveform_isr());
    TIMER_A0->CCR[0] += ISR_PERIOD;
}




