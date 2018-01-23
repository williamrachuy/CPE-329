/*
 * dac.h
 *
 *  Created on: Oct 13, 2017
 *      Author: wrachuy
 */

#ifndef DAC_H_
#define DAC_H_

void dac_init(DIO_PORT_Even_Interruptable_Type* u_cs_port, uint16_t u_cs_pin, EUSCI_B_Type * u_spi);
void dac_drive(uint16_t level);


#endif /* DAC_H_ */
