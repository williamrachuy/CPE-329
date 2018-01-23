/*
 * Header file for the MSP432 driver framework
 * for the nRF24L01+
 * Supports basic functionality of the radio with
 * the Shockburst package specifications.
 *
 * Authors: William Rachuy, Brett Glidden
 */

#ifndef NRF_H_
#define NRF_H_

#include "msp.h"

#define RADIO_1     // Comment out if you are not RADIO_1

#define NRF_CONFIG 0x00
#define NRF_DYNPD 0x1C
#define NRF_FEATURE 0x1D
#define NRF_RF_CH 0x05
#define NRF_STATUS 0x07
#define PRIM_RX 0x01
#define TX_ADDR 0x10
#define RX_ADDR_P0 0x0A
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_ADDR_P1 0x0B
#define EN_RXADDR 0x02
#define ERX_P1 0x02
#define NRF_READ_CMD 0x00
#define NRF_WRITE_CMD 0x20
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2
#define NRF_W_TX_PAYLOAD_NO_ACK 0xB0
#define NRF_R_PAYLOAD 0x61
#define PAYLOAD_SIZE 16
#define RF_SETUP 0x06

uint16_t status;

void NRF_begin();
void NRF_open_writing_pipe(uint8_t *addr);
void NRF_open_reading_pipe(uint8_t pipe, uint8_t *addr);
uint8_t NRF_read_register_8(uint8_t reg);
void NRF_write_register_8(uint8_t reg, uint8_t val);
void NRF_write_register(uint8_t reg, uint8_t *data, uint8_t len);
void NRF_flush_tx();
void NRF_flush_rx();
void NRF_read_payload(uint8_t *data, uint8_t len);
void NRF_write_payload(uint8_t *data, uint8_t len);
void NRF_set_tx();
void NRF_set_rx();
void NRF_ce_high();
void NRF_ce_low();

#endif /* NRF_H_ */
