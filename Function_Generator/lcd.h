#ifndef LCD_H_
#define LCD_H_

#include "msp.h"
#include "clock.h"
#include "delay.h"

void lcd_init();
void lcd_entry_mode_set();
void lcd_clear_display();
void lcd_clear_upper(int ret);
void lcd_clear_lower(int ret);
void lcd_display_control();
void lcd_function_set();
void lcd_return_home();
void lcd_set_address_DDRAM(uint8_t address);
void lcd_set_address_CGRAM(uint8_t address);
void lcd_write_char(char c, uint8_t address);
void lcd_write_string(char *str, uint8_t address);
void lcd_write_pattern(uint8_t address, uint8_t pattern[]);

#endif
