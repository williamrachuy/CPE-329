/* Columns  (3) ... P5.0 - P5.3 ... 0000 0___ ... 0x00 - 0x07 ... First  nibble always 0 */
/* Rows     (4) ... P6.4 - P6.7 ... ____ 0000 ... 0x00 - 0xF0 ... Second nibble always 0 */

#include "keypad.h"

char *keymap[] = {"123","456","789","*0#"};

void keypad_init() {
    P5->DIR &= ~0xF0;                /* columns */

    P5->REN = 0x07;
    P5->OUT = 0x07;

    P6->DIR &= ~(BIT4 + BIT5 + BIT6 + BIT7);                /* rows */
}

char keypad_getkey() {
    int row, col;
    const char row_select[] = {0x10, 0x20, 0x40, 0x80}; /* one row is active */

    /* check to see if any key is pressed */
    P6->DIR |= 0xF0;                /* make all row pins output */
    P6->OUT &= ~0xF0;               /* drive all row pins low */

    delay_us(100, FREQ_48_MHz);     /* wait for signals to settle */

    col = P5->IN & 0x07;            /* read all column pins */

    P6->OUT |= 0xF0;                /* drive all rows high before disabling them */
    P6->DIR &= 0xF0;                /* disable all row pins drive */

    if (col == 0x07) return 0;      /* if all columns are high ... no key pressed */

    /* If a key is pressed, it gets here to find out which key.
     * It activates one row at a time and reads the input to see
     * which column is active. */
    for (row = 0; row < 4; row++) {
        P6->DIR &= 0xF0;                /* disable all rows */
        P6->DIR |= row_select[row];     /* enable one row at a time */
        P6->OUT &= ~row_select[row];    /* drive the active row low */

        delay_us(100, FREQ_48_MHz);     /* wait for signal to settle */

        col = P5->IN & 0x07;            /* read all columns */

        P4->OUT |= row_select[row];     /* drive the active row high */

        if (col != 0x07) break;         /* if one of the inputs is low, some key is pressed */
    }

    P6->OUT |= 0xF0;                    /* drive all rows high before disabling them */
    P6->DIR &= 0xF0;                    /* disable all rows */

    if (row == 4) return 0;             /* if true, no key is pressed */

    /* gets here when one of the row has a key pressed, check which column it is */
    //if (col == 0x06) return row * 4 + 1;    /* key in column 0 */
    //if (col == 0x05) return row * 4 + 2;    /* key in column 1 */
    //if (col == 0x03) return row * 4 + 3;    /* key in column 2 */

    if (col == 0x06) return keymap[row][0];
    if (col == 0x05) return keymap[row][1];
    if (col == 0x03) return keymap[row][2];

    return 0;   /* safety measure */
}
