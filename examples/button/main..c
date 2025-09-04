// button - program to read a pin to demo button presses
// Uses blocking and 5 consecutive reads to confirm a button press
// Lights an LED to indicate a pressed button

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#define LED PB1
#define BUTTON PB0

void main()
{
    // set LED pin to OUTPUT
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (LED));

    // set BUTTON to INPUT PULLUP (set to DDRD to INPUT then set PORTB)
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (BUTTON));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (BUTTON));
    
    for (;;) 
    {
        static uint8_t button_state = 0;
        bool PRESSED = false;
        asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (LED));

        while (~PRESSED)
        {
            // Shift previous states left and add current state
            button_state = (button_state << 1) | (!(PINB & (1 << BUTTON))) | 0xE0;
            // Button is pressed when last 5 readings are all low (pressed)
            if (button_state == 0xF0) 
            {
                PRESSED = true;
                asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (LED));
                _delay_ms(50);
            }
        }
    }
}  
