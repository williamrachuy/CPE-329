/*
 * MSP432 Driver Framework for the nRF24L01+ radio
 * Includes setup and initialization code for register reads,
 * writes, and payload settings.
 *
 * Authors: Brett Glidden, William Rachuy
 */
#include "nrf.h"

static uint8_t NOP_ARRAY[32] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#ifndef RADIO_1
// Five byte address
uint8_t transmit_address[5] = {0x13,0x34,0x43,0x12,0x21};
uint8_t receive_address[5] = {0x31,0x43,0x34,0x21,0x12};
#else
// Five byte address
uint8_t transmit_address[5] = {0x31,0x43,0x34,0x21,0x12};
uint8_t receive_address[5] = {0x13,0x34,0x43,0x12,0x21};
#endif

static DIO_PORT_Even_Interruptable_Type* cs_port = P2;
static DIO_PORT_Even_Interruptable_Type* ce_port = P2;
static uint16_t cs_pin = BIT6;
static uint16_t ce_pin = BIT4;
static EUSCI_B_Type * spi = EUSCI_B2;


/*
 * Drive the CE pin low on the nRF
 */
void NRF_ce_low() {
    ce_port->OUT &= ~ce_pin;
}

/*
 * Drive the CE pin high on the nRF
 */
void NRF_ce_high() {
    ce_port->OUT |= ce_pin;
}

/*
 * Set the nRF in RX mode
 */
void NRF_set_rx() {
    int i;
    // Turn on RX Mode [TODO] -> Clear PIRX bit
    ce_port->OUT &= ~ce_pin;
    // Clear the PIRX bit
    NRF_write_register_8(NRF_CONFIG, NRF_read_register_8(NRF_CONFIG) | (PRIM_RX));
    for(i = 0; i < 3120; i++);    // delay for radio transition state
    ce_port->OUT |= ce_pin;
}

/*
 * Set the nRF in TX mode
 */
void NRF_set_tx() {
    int i;
    // Turn on TX Mode [TODO] -> Clear PIRX bit
    ce_port->OUT &= ~ce_pin;
    // Clear the PIRX bit
    NRF_write_register_8(NRF_CONFIG, NRF_read_register_8(NRF_CONFIG) & ~(PRIM_RX));
    for(i = 0; i < 3120; i++);    // delay for radio transition state
    // NEED TO SET CE HIGH MANUALLY TO TRANSMIT
}

/*
 * Initialize the SPI bus to be used for comms
 * between the MSP and the nRF
 */
void NRF_SPI_init() {
    // Setup slave select and chip enable
    cs_port->SEL0 &= ~(cs_pin);
    cs_port->SEL1 &= ~(cs_pin);
    cs_port->DIR |= cs_pin;
    cs_port->OUT |= cs_pin;

    ce_port->SEL0 &= ~(ce_pin);
    ce_port->SEL1 &= ~(ce_pin);
    ce_port->DIR |= ce_pin;
    ce_port->OUT &= ~ce_pin;

    // Set P2.7 as input for IRQ
    P2->DIR &= ~BIT7;
    P2->SEL0 &= ~BIT7;
    P2->SEL1 &= ~BIT7;
    P2->IES |= BIT7;     // Interrupt on high-to-low
    P2->IFG = 0;
    P2->IE |= BIT7;     // Enable interrupt for P1.1
    NVIC->ISER[1] |= 1 << ((PORT2_IRQn) & 31);

    // Setup radio SPI channel (EUSCI_B2)
    P3SEL0 |= BIT7 + BIT6 + BIT5;
    P3SEL1 &= ~(BIT7 + BIT6 + BIT5);

    spi->CTLW0 |= EUSCI_B_CTLW0_SWRST;   // Put eUSCI state machine in reset

    spi->CTLW0 = EUSCI_B_CTLW0_SWRST |   // Remain eUSCI state machine in reset
                    EUSCI_B_CTLW0_MST   |   // Set as SPI master
                    EUSCI_B_CTLW0_SYNC  |   // Set as synchronous mode
                    EUSCI_B_CTLW0_MSB;      // MSB first

    spi->CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK; // SMCLK
    spi->BRW = 0x02;                         // divide by 2, clock = fBRCLK/(UCBRx + 1) = 12MHz
    spi->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;      // Initialize USCI state machine, SPI

    spi->IFG |= EUSCI_B_IFG_TXIFG;  // Clear TXIFG flag
}

/*
 * Initialization code for the nRF radio with default
 * settings
 */
void NRF_begin() {
    // Configure NRF SPI
    NRF_SPI_init();

    // Force delay for at least 5ms
    int i;
    uint8_t config;
    for(i = 0; i < 128000; i++);

    // Reset NRF_CONFIG, enable 16-bit CRC, disable TX_DS, MAX_RT
    NRF_write_register_8(NRF_CONFIG, 0x38);

    // Disable dynamic payloads
    NRF_write_register_8(NRF_DYNPD, 0x00);

    // Enable W_NO_ACK Command
    NRF_write_register_8(NRF_FEATURE, 0x01);

    // Reset status, clear all IRQ
    NRF_write_register_8(NRF_STATUS, 0x70);

    // Set channel to 76
    NRF_write_register_8(NRF_RF_CH, 33);

    // Set data rate to 1Mbps
    NRF_write_register_8(RF_SETUP, NRF_read_register_8(RF_SETUP) & ~BIT3);

    // Flush RX and TX FIFOs
    NRF_flush_rx();
    NRF_flush_tx();

    // Power up the module
    config = NRF_read_register_8(NRF_CONFIG);
    NRF_write_register_8(NRF_CONFIG, config | BIT1);
    for(i = 0; i < 128000; i++);    // delay for radio transition state

    // Put module in standby-1 mode
    NRF_write_register_8(NRF_CONFIG, NRF_read_register_8(NRF_CONFIG) & (~PRIM_RX));

    // Open a writing and reading pipe
    NRF_open_writing_pipe(transmit_address);
    NRF_open_reading_pipe(1, receive_address);

    // Put radio into RX mode
    ce_port->OUT |= ce_pin;
    NRF_write_register_8(NRF_CONFIG, NRF_read_register_8(NRF_CONFIG) | (PRIM_RX));
}

/*
 * Write a byte to the requested register
 */
void NRF_write_register_8(uint8_t reg, uint8_t val) {
    cs_port->OUT &= ~cs_pin;
    // write to the desired register
    spi->TXBUF = NRF_WRITE_CMD | reg;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;

    spi->TXBUF = val;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;
    cs_port->OUT |= cs_pin;
}

/*
 * Write an arbitrary number of bytes to the requested register
 */
void NRF_write_register(uint8_t reg, uint8_t *data, uint8_t len) {
    cs_port->OUT &= ~cs_pin;
    spi->TXBUF = NRF_WRITE_CMD | reg;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;

    while(len--) {
        spi->TXBUF = *data;
        while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
        // Wait till a character is received
        while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
        // Clear the receive interrupt flag
        spi->IFG &= ~EUSCI_B_IFG_RXIFG;
        data++;
    }
    cs_port->OUT |= cs_pin;
}

/*
 * Read data from the payload FIFO of length 'len'
 */
void NRF_read_payload(uint8_t *data, uint8_t len) {
    uint8_t i;

    cs_port->OUT &= ~cs_pin;
    spi->TXBUF = NRF_R_PAYLOAD;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    status = spi->RXBUF;
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;

    for(i = 0;i < len; i++) {
        spi->TXBUF = NOP_ARRAY[i];
        while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
        // Wait till a character is received
        while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
        data[i] = spi->RXBUF;
        // Clear the receive interrupt flag
        spi->IFG &= ~EUSCI_B_IFG_RXIFG;
    }
    cs_port->OUT |= cs_pin;
}

/*
 * Write 'len' bytes to the payload FIFO for transmit
 */
void NRF_write_payload(uint8_t *data, uint8_t len) {
    uint8_t i;

    cs_port->OUT &= ~cs_pin;
    spi->TXBUF = NRF_W_TX_PAYLOAD_NO_ACK;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    status = spi->RXBUF;
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;

    for(i = 0;i < len; i++) {
        spi->TXBUF = data[i];
        while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
        // Wait till a character is received
        while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
        // Clear the receive interrupt flag
        spi->IFG &= ~EUSCI_B_IFG_RXIFG;
    }
    cs_port->OUT |= cs_pin;
}

/*
 * Read a byte from the requested register
 */
uint8_t NRF_read_register_8(uint8_t reg) {
    uint8_t recv;

    cs_port->OUT &= ~cs_pin;
    // write to the desired register
    spi->TXBUF = NRF_READ_CMD | reg;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    status = spi->RXBUF;
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;

    spi->TXBUF = 0xFF;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    recv = spi->RXBUF;
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;
    cs_port->OUT |= cs_pin;

    return recv;
}

/*
 * Setup a reading pipe at the requested address
 */
void NRF_open_reading_pipe(uint8_t pipe, uint8_t *addr) {
    // Hard coded to pipe 1 for simplicity
    NRF_write_register(RX_ADDR_P1, addr, 5);    // Write addr into RX_ADDR_P(pipe)

    // Write PAYLOAD_SIZE into RX_PW_P(pipe)
    NRF_write_register_8(RX_PW_P1, PAYLOAD_SIZE);

    // Set pipe bit EN_RXADDR
    NRF_write_register_8(EN_RXADDR, ERX_P1);
}

/*
 * Setup a transmit pipe for the requested address
 */
void NRF_open_writing_pipe(uint8_t *addr) {
    // addr width is 5 bytes
    NRF_write_register(RX_ADDR_P0, addr, 5);
    NRF_write_register(TX_ADDR, addr, 5);

    NRF_write_register_8(RX_PW_P0, PAYLOAD_SIZE);
}

/*
 * Flush the TX FIFOs
 */
void NRF_flush_tx() {
    cs_port->OUT &= ~cs_pin;
    spi->TXBUF = FLUSH_TX;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;
    cs_port->OUT |= cs_pin;
}

/*
 * Flush the RX FIFOs
 */
void NRF_flush_rx() {
    cs_port->OUT &= ~cs_pin;
    spi->TXBUF = FLUSH_RX;
    while (!(spi->IFG & EUSCI_B_IFG_TXIFG));
    // Wait till a character is received
    while (!(spi->IFG & EUSCI_B_IFG_RXIFG));
    // Clear the receive interrupt flag
    spi->IFG &= ~EUSCI_B_IFG_RXIFG;
    cs_port->OUT |= cs_pin;
}
