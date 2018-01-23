/* Using P4

    BIT0 -> DB4
     BIT1 -> DB5
      BIT2 -> DB6
       BIT3 -> DB7
        BIT4 -> NONE
         BIT5 -> EN
          BIT6 -> R/W
           BIT7 -> RS

*/

#include "lcd.h"

void pulse_enable() {
    delay_us(30, FREQ_48_MHz);
    P4->OUT |= BIT5;
    delay_us(30, FREQ_48_MHz);
}

void pulse_disable() {
    delay_us(30, FREQ_48_MHz);
    P4->OUT &= ~BIT5;
    delay_us(30, FREQ_48_MHz);
}

void clear_bits() {
    P4->OUT &= ~(BIT7 + BIT6 + BIT3 + BIT2 + BIT1 + BIT0);
    delay_us(2, FREQ_48_MHz);
}

void lcd_init() {
    /* Configure pins as general output */
    P4->SEL0 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT5 | BIT6 | BIT7);
    P4->SEL1 &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT5 | BIT6 | BIT7);
    P4->DIR |= BIT0 | BIT1 | BIT2 | BIT3 | BIT5 | BIT6 | BIT7;
    P4->OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT5 | BIT6 | BIT7);

    source_DCO(FREQ_48_MHz);

    lcd_function_set();
    lcd_display_control();
    lcd_clear_display();
    lcd_entry_mode_set();
    lcd_return_home();
}

void lcd_send(uint8_t command) {
    clear_bits();
    delay_us(50, FREQ_48_MHz);
    pulse_enable();
    P4->OUT |= command;
    pulse_disable();
}

void lcd_set_address_DDRAM(uint8_t address) {
    lcd_send((address >> 4) | 0x08);
    delay_us(60, FREQ_48_MHz);
    lcd_send(address & 0x0F);
    delay_us(60, FREQ_48_MHz);
}

void lcd_set_address_CGRAM(uint8_t address) {
    lcd_send((address >> 4) | 0x04);
    delay_us(60, FREQ_48_MHz);
    lcd_send(address & 0x0F);
    delay_us(60, FREQ_48_MHz);
}

void lcd_write_string(char *str, uint8_t address) {
    while (str && *str) {
        lcd_write_char(*str, address++);
        str++;
    }
}

void lcd_write_char(char c, uint8_t address) {
    lcd_set_address_DDRAM(address);
    lcd_send((c >> 4) | 0x80);
    lcd_send((c & 0x0F) | 0x80);
    delay_us(60, FREQ_48_MHz);
}

void lcd_write_pattern(uint8_t address, uint8_t pattern[]) {
    uint8_t i;

    lcd_set_address_CGRAM(address);

    for (i = 0; i < 8; i++) {
        lcd_send((pattern[i] >> 4) | 0x80);
        lcd_send((pattern[i] & 0x0F) | 0x80);
        delay_us(60, FREQ_48_MHz);
    }
}

void lcd_return_home() {
    lcd_send(0x00);
    lcd_send(0x02);
    delay_ms(3, FREQ_48_MHz);
}

void lcd_entry_mode_set() {
    lcd_send(0x00);
    lcd_send(0x06);
    delay_us(60, FREQ_48_MHz);
}

void lcd_clear_display() {
    lcd_send(0x00);
    lcd_send(0x01);
    delay_ms(3, FREQ_48_MHz);
}

void lcd_clear_upper(int ret) {
    lcd_write_string("                ", 0x00);
    if (ret) {
        lcd_set_address_DDRAM(0x00);
    }
}

void lcd_clear_lower(int ret) {
    lcd_write_string("                ", 0x40);
    if (ret) {
        lcd_set_address_DDRAM(0x40);
    }
}

void lcd_display_control() {
    lcd_send(0x00);
    lcd_send(0x0F);
    delay_us(60, FREQ_48_MHz);
}

void lcd_function_set() {
    lcd_send(0x02);
    lcd_send(0x02);
    lcd_send(0x0C);
    delay_us(60, FREQ_48_MHz);
}

void lcd_clear_dynamic(uint8_t address, uint8_t length) {
    while(length-- > 0) {
        lcd_write_char(' ', address++);
    }
}
