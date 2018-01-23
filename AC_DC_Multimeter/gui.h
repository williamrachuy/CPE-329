#ifndef GUI_H_
#define GUI_H_

#include "msp.h"
#include "vt100.h"

#define DC 0
#define AC 1

void gui_init(EUSCI_A_Type *x, uint16_t interrupt_flag);
void gui_draw_border(void);
void gui_draw_title(void);
void gui_set_mode(uint8_t m);
void gui_graph_display(uint8_t graph_offset, uint8_t filled_buckets);
uint8_t gui_get_mode();
void gui_DC_update(uint32_t value_volt_dc);
void gui_AC_update(uint32_t value_rms, uint32_t value_vpp, uint32_t value_freq);
char *format_number(int32_t value);

#endif
