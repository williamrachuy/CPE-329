#include "msp.h"
#include "clock.h"
#include "delay.h"
#include "lcd.h"
#include "led.h"
#include "keypad.h"
#include "password.h"

#define SKULL 0x00
#define LOCKED 0x00
#define UNLOCKED 0x00
#define CHECKER 0x08

/* Need to move these to lcd.c but I'm getting errors for some reason... */
uint8_t pattern_locked[8] =     {0x0E, 0x11, 0x11, 0x1F, 0x1B, 0x1B, 0x1F, 0x00};
uint8_t pattern_unlocked[8] =   {0x0E, 0x10, 0x10, 0x1F, 0x1B, 0x1B, 0x1F, 0x00};
uint8_t pattern_skull[8] =      {0x0E, 0x1F, 0x1F, 0x04, 0x1B, 0x0E, 0x0A, 0x00};
uint8_t pattern_blackout[8] =   {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
uint8_t pattern_checker[8] =    {0x15, 0x0A, 0x15, 0x0A, 0x15, 0x0A, 0x15, 0x0A};

char reset() {
    lcd_clear_display();
    lcd_return_home();
    lcd_write_string("LOCKED ", 0x00);
    lcd_write_pattern(LOCKED, pattern_locked);
    lcd_write_char(LOCKED, 0x07);
    lcd_write_string("ENTER KEY: ", 0x40);

    return 0;
}

void invalid_key() {
    char i;

    lcd_clear_display();
    lcd_return_home();
    lcd_write_pattern(SKULL, pattern_skull);
    delay_ms(500, FREQ_48_MHz);

    for (i = 0; i < 8; i++) {
        lcd_write_char(SKULL, (0x00 + i*2));
        lcd_set_address_DDRAM(0x50);
        delay_ms(500, FREQ_48_MHz);
    }
    while(1);

    for (i = 0; i < 3; i++) {
        lcd_write_string("INCORRECT!", 0x40);
        lcd_set_address_DDRAM(0x50);
        delay_ms(300, FREQ_48_MHz);
        lcd_clear_lower(0);
        delay_ms(300, FREQ_48_MHz);
    }
}

void main() {
	char flag, i, key, key_follower;
	uint8_t address;
	uint8_t is_locked = 1;

    WDTCTL = WDTPW | WDTHOLD;

    lcd_init();
    keypad_init();
    led_init();

/*
    lcd_write_pattern(LOCKED, pattern_locked);
    lcd_write_pattern(CHECKER, pattern_checker);
*/

    flag = i = key = key_follower = reset();
    address = 0x4B;

    while(1) {
        key_follower = key;
        key = keypad_getkey();

        if ((key_follower == '*') && (key != key_follower)) {
            flag = i = key = key_follower = reset();
            address = 0x4B;
            is_locked = 1;
        }
        else if ((key != '\0') && (key != '*') && (key_follower != key) && is_locked) {
            lcd_write_char(key, address++);

            if (key != password[i++]) {
                flag = 1;
            }

            if (i == 4) {
                //i = reset();
                //address = 0x4B;

                if (!flag) {
                    lcd_clear_display();
                    lcd_return_home();
                    lcd_write_string("HELLO WORLD! ", 0x00);
                    lcd_write_pattern(UNLOCKED, pattern_unlocked);
                    lcd_write_char(UNLOCKED, 0x0D);
                    lcd_set_address_DDRAM(0x50);
                    is_locked = 0;
                }
                else {
                    flag = i = reset();
                    delay_ms(50, FREQ_48_MHz);
                    invalid_key();
                    flag = i = reset();
                    address = 0x4B;
                    is_locked = 1;
                }

                address = 0x4B;
            }
        }
    }
}
