/*
 * dac.h
 *
 *  Created on: Oct 13, 2017
 *      Author: wrachuy
 */

#ifndef DAC_H_
#define DAC_H_

void init_dac(DIO_PORT_Even_Interruptable_Type* u_cs_port, uint16_t u_cs_pin, EUSCI_B_Type * u_spi);
void drive_dac(uint16_t level);


#endif /* DAC_H_ */
