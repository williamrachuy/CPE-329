#include "msp.h"
#include "frequency.h"

static int16_t freq_loops = 0;
static int64_t freq_add = 0;
static uint8_t freq_sampled = 0;

void frequency_timer_init() {
    // ACLK to 128KHz
    CS->KEY = CS_KEY_VAL;
    CS->CTL1 |= CS_CTL1_SELA_2;
    CS->CLKEN |= CS_CLKEN_REFO_EN | CS_CLKEN_REFOFSEL;
    CS->KEY = 0;

    // Setup comparator inputs
    P5->SEL0 |= BIT6 + BIT7;
    P5->SEL1 |= BIT6 + BIT7;

    // Setup comparator ouput
    P7->DIR |= BIT2;
    P7->SEL0 |= BIT2;

    // Setup timer catpure i/o
    P7->DIR &= ~BIT3;
    P7->SEL0 |= BIT3;
    P7->SEL1 &= ~BIT3;

    // TIMER_A0 128kHz frequency timer
    TIMER_A0->CTL |= TIMER_A_CTL_IE;                                // Overflow interrupt enabled
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CAP | TIMER_A_CCTLN_CM_1 |
         TIMER_A_CCTLN_CCIS_0 | TIMER_A_CCTLN_SCCI | TIMER_A_CCTLN_SCS |
         TIMER_A_CCTLN_CCIE;                                        // Capture interrupt enabled
    TIMER_A0->CTL |= TIMER_A_CTL_TASSEL_1 | // ACLK
         TIMER_A_CTL_MC_2 | TIMER_A_CTL_ID_0;                       // continuous mode, 128kHz

    // TIMER_A1 32kHz 1s, 1ms timer
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE;                        // TACCR0 interrupt enabled
    TIMER_A1->CCTL[1] |= TIMER_A_CCTLN_CCIE;
    TIMER_A1->CCR[0] = 32;
    TIMER_A1->CCR[1] = 32000;
    TIMER_A1->CTL |= TIMER_A_CTL_TASSEL_1 |                         // ACLK
         TIMER_A_CTL_MC_2 | TIMER_A_CTL_ID_2;                       // continues mode, 32kHz

    // Comparator setup
    COMP_E1->CTL0 |= COMP_E_CTL0_IMEN |     // Enable V-, input channel C1.6
            COMP_E_CTL0_IMSEL_6;
    COMP_E1->CTL0 |= COMP_E_CTL0_IPEN |     // Enable V+, input channel C1.7
            COMP_E_CTL0_IPSEL_7;
    COMP_E1->CTL1 |= COMP_E_CTL1_PWRMD_0 | COMP_E_CTL1_F | COMP_E_CTL1_FDLY_3;    // normal power mode
    COMP_E1->CTL2 |= COMP_E_CTL2_CEREFL_0 |                             // No reference supplied to (-)
            COMP_E_CTL2_RS_0 | COMP_E_CTL2_RSEL;
    COMP_E1->CTL1 |= COMP_E_CTL1_ON;            // Turn On Comparator_E

    freq_period = 0;

    NVIC->ISER[0] |= 1 << ((TA0_0_IRQn) & 31);
    NVIC->ISER[0] |= 1 << ((TA0_N_IRQn) & 31);
    NVIC->ISER[0] |= 1 << ((TA1_0_IRQn) & 31);
    NVIC->ISER[0] |= 1 << ((TA1_N_IRQn) & 31);
}

// Frequency update timer
void TA0_0_IRQHandler(void) {
    __disable_irq();

    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    if (freq_sampled) {
       if (((freq_loops*0xFFFF) + TIMER_A0->CCR[0] - freq_add) > 120) {
           freq_period = (freq_loops*0xFFFF) + TIMER_A0->CCR[0] - freq_add;
       }
       freq_sampled &= 0x00;
       freq_loops &= 0;
       freq_add &= 0;
       TIMER_A0->R &= 0x00;

    } else {
       // Get initial offset
       freq_add = (freq_loops*0xFFFF) + TIMER_A0->CCR[0];
       freq_sampled++;
    }

    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_COV;
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    __enable_irq();
}

// Frequency overflow timer
void TA0_N_IRQHandler(void) {
    __disable_irq();
    TIMER_A0->CTL &= ~TIMER_A_CTL_IFG;
    freq_loops++;
    __enable_irq();
}
