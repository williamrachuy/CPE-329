#include "msp.h"
#include "dac.h"


static DIO_PORT_Even_Interruptable_Type* cs_port;
static uint16_t cs_pin;
static EUSCI_B_Type * spi;

/* Function: configure the CS and MISO/MOSI pins to use for the DAC
 * TODO: implement checks
 */
void init_dac(DIO_PORT_Even_Interruptable_Type* u_cs_port, uint16_t u_cs_pin, EUSCI_B_Type * u_spi) {
    cs_pin = u_cs_pin;
    cs_port = u_cs_port;
    spi = u_spi;

    // CS setup
    cs_port->SEL0 &= ~cs_pin;
    cs_port->SEL1 &= ~cs_pin;
    cs_port->DIR |= cs_pin;
    cs_port->OUT |= cs_pin;

    P1SEL0 |= BIT6 + BIT5; // Configure P1.6 and P1.5 for UCB0SIMO and UCB0CLK
    P1SEL1 &= ~(BIT6 + BIT5); //

    // SPI Setup
    spi->CTLW0 |= EUSCI_B_CTLW0_SWRST;   // Put eUSCI state machine in reset

    spi->CTLW0 = EUSCI_B_CTLW0_SWRST |   // Remain eUSCI state machine in reset
                    EUSCI_B_CTLW0_MST   |   // Set as SPI master
                    EUSCI_B_CTLW0_SYNC  |   // Set as synchronous mode
                    EUSCI_B_CTLW0_CKPL  |   // Set clock polarity high
                    EUSCI_B_CTLW0_MSB;      // MSB first

    spi->CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK; // SMCLK
    spi->BRW = 0x01;                         // divide by 16, clock = fBRCLK/(UCBRx)
    spi->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;      // Initialize USCI state machine, SPI
                                                // now waiting for something to
                                                // be placed in TXBUF

    spi->IFG |= EUSCI_B_IFG_TXIFG;  // Clear TXIFG flag
}

void drive_dac(uint16_t level) {
    uint16_t dac_word = 0x00;
    int i = 0;

    dac_word = (0x3000) | (level & 0x0FFF);

    cs_port->OUT &= ~cs_pin;

    spi->TXBUF = (unsigned char) (dac_word >> 8);
    while(!(spi->IFG & EUSCI_B_IFG_TXIFG));

    spi->TXBUF = (unsigned char) (dac_word & 0x00FF);
    while(!(spi->IFG & EUSCI_B_IFG_TXIFG));

    // possible software delay loop here
    for(i=0;i<0;i++);

    cs_port->OUT |= cs_pin;

    return;
}
