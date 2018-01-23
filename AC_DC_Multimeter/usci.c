#include "msp.h"
#include "usci.h"

void usci_init() {
    P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
            EUSCI_B_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK

                                            // Baud Rate calculation
                                            // 12000000/(16*9600) = 78.125
                                            // Fractional portion = 0.125
                                            // User's Guide Table 21-4: UCBRSx = 0x10
                                            // UCBRFx = int ( (78.125-78)*16) = 2
    EUSCI_A0->BRW = 26;                     // 12000000/16/115200
    EUSCI_A0->MCTLW = (1 << EUSCI_A_MCTLW_BRF_OFS) | EUSCI_A_MCTLW_OS16 | (0xB7 << EUSCI_A_MCTLW_BRS_OFS) ;

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        // Enable USCI_A0 RX interrupt

    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);

}
