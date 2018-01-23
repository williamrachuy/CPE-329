#include "clock.h"

/* change the DCO frequency and set it to the master clock */
void source_DCO (int source) {
    if (source) {
        if (source == FREQ_48_MHz) {
            while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
            PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR_1;
            while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));

            FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) | FLCTL_BANK0_RDCTL_WAIT_1;
            FLCTL->BANK1_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK1_RDCTL_WAIT_MASK)) | FLCTL_BANK1_RDCTL_WAIT_1;
        }

        CS->KEY = CS_KEY_VAL;
        CS->CTL0 = 0;

        switch (source) {
        case FREQ_15_MHz:
            CS->CTL0 = CS_CTL0_DCORSEL_0;
            CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
            break;
        case FREQ_03_MHz:
            CS->CTL0 = CS_CTL0_DCORSEL_1;
            CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
            break;
        case FREQ_06_MHz:
            CS->CTL0 = CS_CTL0_DCORSEL_2;
            CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
            break;
        case FREQ_12_MHz:
            CS->CTL0 = CS_CTL0_DCORSEL_3;
            CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
            break;
        case FREQ_24_MHz:
            CS->CTL0 = CS_CTL0_DCORSEL_4;
            CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
            break;
        case FREQ_48_MHz:
            CS->CTL0 = CS_CTL0_DCORSEL_5;
            CS->CTL1 = (CS->CTL1 & ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK)) | CS_CTL1_SELM_3;
            break;
        default:
            break;
        }

        CS->KEY = 0;
    }
}

int get_current_DCO() {
    switch(CS->CTL0) {
        case CS_CTL0_DCORSEL_1:
            return FREQ_03_MHz;
        case CS_CTL0_DCORSEL_2:
            return FREQ_06_MHz;
        case CS_CTL0_DCORSEL_3:
            return FREQ_12_MHz;
        case CS_CTL0_DCORSEL_4:
            return FREQ_24_MHz;
        case CS_CTL0_DCORSEL_0:
            return FREQ_15_MHz;
        case CS_CTL0_DCORSEL_5:
            return FREQ_48_MHz;
        default:
            return 0;
    }
}
