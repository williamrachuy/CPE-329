/*
 * CPE 329 Final Project: Walkie Talkie
 * Authors: William Rachuy, Brett Glidden
 *
 * Description: This project contains code to build
 *  a basic walkie talkie system using an MCP4921 DAC
 *  and nRF24L01+ wireless transceiver. The entire
 *  control logic is interrupt driven to save power.
 */

#include "msp.h"
#include "dac.h"
#include "nrf.h"

#define SEND 1
#define RECV 0
uint8_t NRF_mode;

#define PTT_ON 1
#define PTT_OFF 0
uint8_t PTT_state;

uint8_t remaining_samples = 0;  // TIMER_A2 IRQ sample counter
uint8_t sample_count = 0;       // TIMER_A1 IRQ sample counter
uint16_t mic_samples[8];        // 12-bit samples from the Microphone
uint16_t incoming_tx[8];        // 32 byte payload data

uint8_t send_it = 0;            // For debugging to test sending speed
uint8_t recv_it = 0;            // For debugging to test receiving speed

void TIMERS_init();
void ADC_init();

uint16_t nrf_config;

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

    // Set ACLK to 128kHz, SMCLK to 24MHz
    CS->KEY = CS_KEY_VAL;
    CS->CTL1 |= CS_CTL1_SELA_2;
    CS->CTL0 = CS_CTL0_DCORSEL_4;
    CS->CTL1 |= CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
    CS->CLKEN |= CS_CLKEN_REFO_EN | CS_CLKEN_REFOFSEL;
    CS->KEY = 0;

    // Set P1.1 as input with REN high (button)
    P1->DIR &= ~BIT1;
    P1->REN |= BIT1;
    P1->OUT |= BIT1;
    P1->SEL0 &= ~BIT1;
    P1->SEL1 &= ~BIT1;
    P1->IES = BIT1;     // Interrupt on high-to-low
    P1->IFG = 0;
    P1->IE |= BIT1;     // Enable interrupt for P1.1
    NVIC->ISER[1] = 1 << ((PORT1_IRQn) & 31);
    PTT_state = PTT_OFF;

    // PAM8302 SHTDN pin is driven high to turn amplifier on
    P5->SEL0 &= ~BIT6;
    P5->SEL1 &= ~BIT6;
    P5->DIR |= BIT6;
    P5->OUT |= BIT6;

    // begin peripheral initialization
    DAC_init();
    TIMERS_init();
    ADC_init();
    NRF_begin();

    // sanity check
    nrf_config = NRF_read_register_8(NRF_CONFIG);

    // Enable global interrupts
    __enable_irq();

    while(1) {
        __sleep();
    }
}

// DAC driving timer interrupt
void TA2_0_IRQHandler(void) {
    __disable_irq();
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A2->CCR[0] += 1;

    DAC_drive(incoming_tx[remaining_samples++]);

    remaining_samples++;

    if (remaining_samples == 8) {
        TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
        remaining_samples = 0;
    }
    __enable_irq();
}

// IRQ for nRF24L01+ in receive mode
void PORT2_IRQHandler(void) {
    __disable_irq();
    volatile uint32_t i;
    if (P2->IFG & BIT7) {

        recv_it++;
        int len = 0;

        // Read the payload data
        NRF_read_payload((uint8_t *)incoming_tx, 16);

        // Clear the RX_DS register
        NRF_write_register_8(NRF_STATUS, (NRF_read_register_8(NRF_STATUS) | (BIT6)));
        NRF_mode = RECV;

        // Start the output timer for dac driving
        TIMER_A2->CCR[0] = TIMER_A2->R + 1;
        TIMER_A2->CCTL[0] |= TIMER_A_CCTLN_CCIE;
    }
    P2->IFG &= ~BIT7;
    __enable_irq();
}

// Input ADC sampling bulk reader interrupt
void ADC14_IRQHandler(void)
{
    __disable_irq();
    mic_samples[sample_count++] = ADC14->MEM[0] >> 2;
    if (sample_count == 8) {
        sample_count = 0;

        // cast payload data to uint8_t with 2*8 for length
        NRF_write_payload((uint8_t *)mic_samples, 16);

        NRF_mode = SEND;
        NRF_ce_high();
        send_it++;
    }
    __enable_irq();
}

// Input sampling timer interrupt
void TA1_0_IRQHandler(void) {
    __disable_irq();
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A1->CCR[0] += 1;

    /* Force ADC to sample */
    ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;
    __enable_irq();
}

// PTT interrupt
void PORT1_IRQHandler(void) {
    volatile uint32_t i;
    if (P1->IFG & BIT1) {
        // Delay for switch debounce
        for(i = 0; i < 10000; i++);
        if (PTT_state == PTT_OFF) {
            NRF_set_tx();

            TIMER_A1->CCR[0] = TIMER_A1->R + 1;
            TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE;
            PTT_state = PTT_ON;
            P1->IES &= ~BIT1;       // wait for low-to-high transition
        } else {
            // PTT is off, disable sampling timer
            TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
            PTT_state = PTT_OFF;
            NRF_set_rx();

            P1->IES |= BIT1;       // wait for high-to-low transition
        }
    }
    P1->IFG &= ~BIT1;
}

// Initialize the ADC for P5.4 to sample microphone
void ADC_init() {
    // GPIO Setup
    P5->SEL1 |= BIT4;                       // Configure P5.4 for ADC
    P5->SEL0 |= BIT4;

    // Sampling time, S&H=16, ADC14 on
    ADC14->CTL0 = ADC14_CTL0_PDIV_0 | ADC14_CTL0_SHT0_2 | ADC14_CTL0_SHP | ADC14_CTL0_SSEL__SMCLK | ADC14_CTL0_ON;
    ADC14->CTL1 = ADC14_CTL1_RES_3;         // Use sampling timer, 12-bit conversion results

    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_1;   // A1 ADC input select; Vref=AVCC
    ADC14->IER0 |= ADC14_IER0_IE0;          // Enable ADC conv complete interrupt

    // Enable ADC interrupt in NVIC module
    NVIC->ISER[0] |= 1 << ((ADC14_IRQn) & 31);
}

// Initialize two 12kHz timers for input and output
void TIMERS_init() {
    // Mic sampling timer
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A1->CCR[0] = 8000;
    TIMER_A1->CTL |= TIMER_A_CTL_TASSEL_1 |                         // ACLK
         TIMER_A_CTL_MC_2 | TIMER_A_CTL_ID_2;                       // continues mode, 32kHz

    NVIC->ISER[0] |= 1 << ((TA1_0_IRQn) & 31);

    // DAC driving timer
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A2->CCR[0] = 8000;
    TIMER_A2->CTL |= TIMER_A_CTL_TASSEL_1 |                         // ACLK
         TIMER_A_CTL_MC_2 | TIMER_A_CTL_ID_2;                       // continues mode, 32kHz

    NVIC->ISER[0] |= 1 << ((TA2_0_IRQn) & 31);
}
