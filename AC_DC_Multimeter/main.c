#include "msp.h"
#include "clock.h"
#include "usci.h"
#include "adc14.h"
#include "gui.h"
#include "frequency.h"
#include "ac_mode.h"
#include "dc_mode.h"
#include "dac.h"
#include <math.h>

uint32_t test_dc = 10485;
uint32_t test_rms = 10485;
uint32_t test_vpp = 10485;
uint32_t test_freq = 1000;

uint8_t dc_flag = 0;
uint8_t ac_flag = 0;
uint8_t testflag = 0;


void main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    source_DCO(FREQ_48_MHz);            // S

    // EUSCI_A setup 115200 Baud
    usci_init();
    adc14_init();
    dac_init(P2, BIT5, EUSCI_B0);
    gui_init(EUSCI_A0, EUSCI_A_IFG_TXIFG);
    frequency_timer_init();
    dc_mode_init();
    ac_mode_init();
    adc14_sample();

    while(1) {
        if (dc_flag) {
            __disable_irq();
            dc_flag &= 0x00;
            gui_DC_update(dc_average_s);
            __enable_irq();
        } else if (ac_flag) {
            __disable_irq();
            ac_flag &= 0x00;
            gui_AC_update(average_rms, max-min, frequency);
            min = 0xFFFF;
            max = 0x0000;
            rms = 0;
            __enable_irq();
        }
    }
}

void EUSCIA0_IRQHandler(void) {
    char RX;

    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) {
        EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;
        while (!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));

        RX = EUSCI_A0->RXBUF;

        if (RX == 'H') {
            gui_draw_border();
            gui_draw_title();
        }
        else if (RX == 0x09) {
            gui_set_mode(gui_get_mode() ^ 0x01);
        }
        else if (RX == 'C') {
            if (test_freq < 1500000) {
                test_dc += 100;
                test_rms += 100;
                test_vpp += 100;
                test_freq += 10000;
            }
            else {
                test_dc = test_rms = test_vpp = test_freq = 0;
            }
            gui_AC_update(test_rms, test_vpp, test_freq);
        }
        else
            EUSCI_A0->TXBUF = RX;
    }
}

// 1 ms interrupt
void TA1_0_IRQHandler(void) {
    __disable_irq();

    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A1->CCR[0] += 32;

    dc_sum_s += dc_sum_ms;
    dc_samples_s += dc_samples_ms;
    dc_sum_ms = dc_samples_ms = 0;
    testflag |= 0x01;
    __enable_irq();
}

// 1 s interrupt
void TA1_N_IRQHandler(void) {
    __disable_irq();

    if (TIMER_A1->CCTL[1] & TIMER_A_CCTLN_CCIFG) {
        TIMER_A1->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG;
        if(gui_get_mode() == AC) {
            average_rms = isqrt64(rms/rms_samples);
            frequency = 128000000 / freq_period;
            ac_flag |= 0x01;
            rms = rms_samples = 0;
        } else {
            if(testflag && dc_samples_s > 50) {
                dc_average_s = dc_sum_s / dc_samples_s;
                dc_flag |= 1;
                //gui_DC_update(dc_average_s);
                testflag &= 0x00;
                dc_sum_s = dc_samples_s = 0;
            }
        }
        TIMER_A1->CCR[1] += 65000;
    }

    __enable_irq();
}



// ADC14 interrupt service routine
void ADC14_IRQHandler(void) {
    __disable_irq();

    adc14_temp = ADC14->MEM[0];

    dc_sum_ms += adc14_temp;
    dc_samples_ms++;

    if (adc14_temp < min)
        min = adc14_temp;
    if (adc14_temp > max)
        max = adc14_temp;

    //rms += adc14_temp*adc14_temp;
    rms += ((uint64_t)adc14_temp)*((uint64_t)adc14_temp);
    rms_samples++;
    dac_drive(adc14_temp >> 2);

    //adc14_data_set(ADC14->MEM[0]);
    //dc_sum_ms_update(adc14_temp);
    //dc_sum_s_sqr_update(adc14_temp);
    //ac_min_max_update(adc14_temp);


    adc14_sample();
    __enable_irq();

}
