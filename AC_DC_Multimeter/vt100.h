/*
 * vt100.h
 *
 *  Created on: Oct 31, 2017
 *      Author: bglidden
 */

#ifndef VT100_H_
#define VT100_H_

#include "msp.h"

void vt100_init(EUSCI_A_Type * x, uint16_t interrupt_flag);
void vt100_send_cmd(char* payload);
void vt100_set_cursor(int row, int column);
void vt100_clear_screen(void);
char *itoa(int i);
void vt100_write_char(char c);
uint8_t vt100_write_string(char *str);
void gui_DC_display(void);
void gui_AC_display(void);

#endif /* VT100_H_ */
