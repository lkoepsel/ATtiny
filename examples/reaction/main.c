// reaction - simple reaction time game
// Game Rules
// 1. Light YELLOW between .5 and 2 seconds
// 2. Blink GREEN to start.
// 3. User must press button to match the time of the YELLOW  
// 4. GREEN if acceptable
// 5. RED if not acceptable
// 6. Five chances, at the end, blink GREEN for successful turns and blink RED for unsuccessful

//
//                     ATtiny13
//                   ┌──────────┐
//     RESET/PB5 ──1─┤          ├─8── VCC
//           PB3 ──2─┤          ├─7── PB2/WHITE
//    BUTTON/PB4 ──3─┤          ├─6── PB1/BLUE
//           GND ──4─┤          ├─5── PB0/YELLOW
//                   └──────────┘
//

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "ATtiny.h"

// Define hardware, YELLOW/BLUE/YELLOW/BUTTON
#define YELLOW PB0
#define BLUE PB1
#define WHITE PB2
#define BUTTON PB4
#define LED_DUR 100
#define LED_x2 LED_DUR*2
#define LED_x3 LED_DUR*3
#define LED_x4 LED_DUR*4
#define LED x5 LED_DUR*5
#define ALLOW LED_DUR/4

// ****Defined Interrupt Service Routines****
volatile uint8_t ticks_ctr = 0;

// Required for ticks timing, see examples/ticks
// Enabled by init_sysclock_1() in sysclock.c
ISR (TIM0_COMPA_vect)      
{
    ticks_ctr++;
    // Use to check frequency, currently 250Hz
    // SBI(PINB, PB3);
}

// ****End of Defined Interrupt Service Routines****

// Initialize timer to 250 ticks for ~5 seconds (1 tick = 20ms)
void init_sysclock_100 (void)          
{
    // Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
    // The values below will result in a 1kHz counter (500 ticks = 1 second)
    // CTC mode (WGM0[2:0] = 2)
    // Set clock select to /64 CS => 011
    // Bit 2 – OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
    // OCR0A = x23
    // TC0 Register Values
    // `tc0`, `tccr0a`, "TCCR0A", 0x0000004F, 8-bit | 0x42 (66, 0b01000010), WGM0: 0b10, COM0B: 0b00, COM0A: 0b01
    // `tc0`, `tcnt0`, "TCNT0", 0x00000052, 8-bit | 0x09 (9, 0b00001001)
    // `tc0`, `tccr0b`, "TCCR0B", 0x00000053, 8-bit | 0x03 (3, 0b00000011), CS0: 0b011, WGM02: 0b0, FOC0B: 0b0, FOC0A: 0b0
    // `tc0`, `ocr0a`, "OCR0A", 0x00000056, 8-bit | 0x23 (35, 0b00100011)
    // `tc0`, `tifr0`, "TIFR0", 0x00000058, 8-bit | 0x08 (8, 0b00001000), TOV0: 0b0, OCF0A: 0b0, OCF0B: 0b1
    // `tc0`, `timsk0`, "TIMSK0", 0x00000059, 8-bit | 0x04 (4, 0b00000100), TOIE0: 0b0, OCIE0A: 0b1, OCIE0B: 0b0

    TCCR0A = ( _BV(COM0A0) | _BV(WGM01) ) ; 
    TCCR0B |= ( _BV(CS01) | _BV(CS00) ) ;
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 0x23;
    sei();
 }

// ****End of Defined Timer Setup Functions****

uint8_t ticks(void) {
    return(ticks_ctr);
}

int main (void)
{
    // Initialize timer to 255 ticks for 2.55 seconds (1 tick = 10ms)
    init_sysclock_100 ();
    // temp pin for determing freq
    // DDRB |= (_BV(PB3));

    /* setup LEDs */
    DDRB |=( _BV(BLUE) | _BV(WHITE) | _BV(YELLOW));

    // setup BUTTON to INPUT PULLUP (set to DDRD to INPUT then set PORTB)
    CBI(DDRB, BUTTON);
    SBI(PORTB, BUTTON);

    // Blink all three LEDs to indicate game will start
    PORTB |= ( _BV(BLUE) | _BV(WHITE) | _BV(YELLOW));
    _delay_ms(500);
    PORTB &= ~( _BV(BLUE) | _BV(WHITE) | _BV(YELLOW));
    _delay_ms(1000);

    for (;;) 
    {
        // show duration of the 5 blinks, from longest to shortest
        uint8_t i = 5;
        uint8_t start = 0;
        uint8_t end = 0;
        uint8_t delta = 0;
        do 
        {
            // Light BLUE a LED_TIME between .5 and 2.5 seconds
            SBI(PORTB, BLUE);
            volatile uint8_t LED_TIME = 0;
            uint8_t j = i;
            do
            {
                _delay_ms(LED_DUR);
                LED_TIME += LED_DUR;
            } while (--j);

            CBI(PORTB, BLUE);
            _delay_ms(1000);
        } while (--i);

        // Start timer
        start = ticks();
        
        // When button is pressed, determine PRESS_TIME
        static uint8_t button_state = 0;
        bool PRESSED = false;
        CBI (PORTB, WHITE);

        while (!PRESSED)
        {
            // Shift previous states left and add current state
            button_state = (button_state << 1) | (!(PINB & (1 << BUTTON))) | 0xE0;
            // Button is pressed when last 5 readings are all low (pressed)
            if (button_state == 0xF0) 
            {
                PRESSED = true;
                SBI(PORTB, WHITE);
                end = ticks();
                _delay_ms(50);
            }
        }
            
        // Compare PRESS_TIME to LED_TIME
        if (start > end)
        {
            delta = start - end;
        }
        else
        {
            delta = end - start;
        }

        // If CLOSE, blink BLUE else, blink YELLOW
        if ((delta < (LED_DUR + ALLOW)) & (delta > (LED_DUR - ALLOW)))
        {
            SBI(PORTB, BLUE);
            _delay_ms(500);
            CBI(PORTB, BLUE);
            _delay_ms(500);
        }
        // Blink YELLOW for every failure
        else
        {
            SBI(PORTB, YELLOW);
            _delay_ms(500);
            CBI(PORTB, YELLOW);
            _delay_ms(500);
        }       
    
        // Repeat 4 more times, with variable LED_TIME

    };
}
