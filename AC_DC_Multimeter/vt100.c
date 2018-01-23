/*
 * vt100.c
 *
 *  Created on: Oct 31, 2017
 *      Author: bglidden
 */

#include "vt100.h"

static EUSCI_A_Type * serial;
static uint16_t IFG;

void vt100_init(EUSCI_A_Type *x, uint16_t interrupt_flag) {
    serial = x;
    IFG = interrupt_flag;
}

/* Function: send cmd
 * Desc: sends the designated payload to over the UART. Includes
 *       the <ESC> and '[' characters.
 */
void vt100_send_cmd(char* payload) {

    // wait for any previous activity on UART
    while(!(EUSCI_A0->IFG & IFG));

    serial->TXBUF = 0x1B;   // send <ESC>
    while(!(serial->IFG & IFG));

    serial->TXBUF = 0x5B;   // send [
    while(!(serial->IFG & IFG));

    // send the command data
    while (payload && *payload) {
        serial->TXBUF = *payload;
        payload++;
        while(!(serial->IFG & IFG));
    }
    return;
}

/* Function: set cursor
 * Desc: sets the cursor to the coordinates specified by the row and
 *       column variables.
 */
void vt100_set_cursor(int row, int column) {
   char buf[50];
   uint8_t i = 0;
   char *temp;

   temp = itoa(row);
   while (temp && *temp) {
       buf[i++] = *temp;
       temp++;
   }

   buf[i++] = ';';
   temp = itoa(column);
   while (temp && *temp) {
       buf[i++] = *temp;
       temp++;
   }
   buf[i++] = 'H';
   buf[i++] = '\0';

   vt100_send_cmd(buf);

}

/* Function: clear screen
 * Desc: Clears the entire screen by setting the cursor to upper left
 *       and then executing the clear screen command.
 */
void vt100_clear_screen(void) {
    vt100_set_cursor(0,0);
    vt100_send_cmd("J");
}

void vt100_draw_border(int width, int height) {
    int i, j;
    vt100_clear_screen();
    vt100_set_cursor(1,2);

    // Draw top border
    for(i = 2; i < width; i++) {
        while(!(serial->IFG & IFG));
        serial->TXBUF = '_';
    }

    // Draw horizontal borders
    for (i = 2; i < height + 2; i++) {
        vt100_set_cursor(i, 0);
        while(!(serial->IFG & IFG));
        serial->TXBUF = '|';
        vt100_set_cursor(i, width);
        while(!(serial->IFG & IFG));
        serial->TXBUF = '|';
    }

    vt100_set_cursor(height + 1,2);
    for(j = 2; j < width; j++) {
        while(!(serial->IFG & IFG));
        serial->TXBUF = '_';
    }

}

/* Function: itoa
 * Desc: Converts integer values to ASCII strings
 */
char *itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[21];
  char *p = buf + 19 + 1;   /* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {            /* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

void vt100_write_char(char c) {
	while(!(serial->IFG & IFG));
	serial->TXBUF = c;
}

uint8_t vt100_write_string(char *str) {
    uint8_t i = 0;

	while(str && *str) {
		while(!(serial->IFG & IFG));
        serial->TXBUF = *str;
		str++;
		i++;
	}

	return i;
}
