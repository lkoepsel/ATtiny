// reaction - simple reaction time game
// Game Rules
// 1. Light BLUE between .5 and 2 seconds
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
#include <inttypes.h>
#include "ATtiny.h"

// Define hardware, YELLOW/BLUE/WHITE/BUTTON
#define YELLOW PB0
#define BLUE PB1
#define WHITE PB2
#define BUTTON PB4
#define LED_DUR 250     // increment LED time by 1/4 of a second
#define ALLOW 10        // allow for a variance of 10 ticks or .25s

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

// Initialize timer to ~40 ticks for 1 seconds (1 tick = 25ms or max = ~6.5)
void init_sysclock_100 (void)          
{
    // Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
    // The values below will result in a ~62.5Hz counter (40 ticks = 1 second)
    // CTC mode (WGM0[2:0] = 2)
    // Set clock select to /256 CS => 100
    // Bit 2 – OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
    // OCR0A = x23
    // TC0 Register Values
    // "GTCCR", 0x00000048, 0x00 (0, 0b00000000), PSR10: 0b0, TSM: 0b0
    // "OCR0B", 0x00000049, 0x00 (0, 0b00000000)
    // "TCCR0A", 0x0000004F, 0x02 (2, 0b00000010), WGM0: 0b10, COM0B: 0b00, COM0A: 0b00
    // "TCNT0", 0x00000052, 0x12 (18, 0b00010010)
    // "TCCR0B", 0x00000053, 0x04 (4, 0b00000100), CS0: 0b100, WGM02: 0b0, FOC0B: 0b0, FOC0A: 0b0
    // "OCR0A", 0x00000056, 0x23 (35, 0b00100011)
    // "TIFR0", 0x00000058, 0x08 (8, 0b00001000), TOV0: 0b0, OCF0A: 0b0, OCF0B: 0b1
    // "TIMSK0", 0x00000059, 0x04 (4, 0b00000100), TOIE0: 0b0, OCIE0A: 0b1, OCIE0B: 0b0
    TCCR0A = ( _BV(WGM01) ) ; 
    TCCR0B |= ( _BV(CS02) ) ;
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 0x75;
    sei();
 }

// ****End of Defined Timer Setup Functions****

int main (void)
{
    // initialize the pseudorandom number generator
    volatile uint8_t state = 0xE1;

    // Initialize timer to 255 ticks for 2.55 seconds (1 tick = 10ms)
    init_sysclock_100 ();
    // temp pin for determing freq
    DDRB |= (_BV(PB3));

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
        uint8_t i = 3;
        uint8_t good = 0;
        do
        {
            // show duration of the 5 blinks, from longest to shortest
            uint8_t button_start = 0;
            uint8_t button_end = 0;
            uint8_t button_delta = 0;
            volatile uint8_t led_delta = 0;
            // uint8_t j = i*4;

            // Light BLUE a led_start between .5 and 2.5 seconds
            SBI(PORTB, BLUE);
            uint8_t led_start = ticks_ctr;
            state ^= state << 1;
            state ^= state >> 1;
            state ^= state << 2;
            uint16_t i = state;
            do
            {
            _delay_ms(25);
            } while(--i);
            // do
            // {
            //     _delay_ms(LED_DUR);
            // } while (--j);
            uint8_t led_end = ticks_ctr;
            if (led_start > led_end)
            {
                led_delta = (255 - led_start) + led_end;
            }
            else
            {
                led_delta = led_end - led_start;
            }

            CBI(PORTB, BLUE);
            _delay_ms(500);

            // button_start timer
            button_start = ticks_ctr;
            
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
                    button_end = ticks_ctr;
                }
            }
                
            // Compare PRESS_TIME to led_start
            if (button_start > button_end)
            {
                button_delta = (255 - button_start) + button_end;
            }
            else
            {
                button_delta = button_end - button_start;
            }

            _delay_ms(500);
            // If CLOSE, blink BLUE else, blink YELLOW
            if ((button_delta < (led_delta + ALLOW)) & (button_delta > (led_delta - ALLOW)))
            {
                SBI(PORTB, WHITE);
                _delay_ms(500);
                CBI(PORTB, WHITE);
                ++good;
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
        
            // Repeat 4 more times, with variable led_start
        } while (--i);

        if (good == 3)
        {
            uint8_t i = 3;
            do
            {
                PORTB |= ( _BV(BLUE) | _BV(WHITE) | _BV(YELLOW));
                _delay_ms(500);
                PORTB &= ~( _BV(BLUE) | _BV(WHITE) | _BV(YELLOW));
                _delay_ms(1000);
            } while ( --i);
        }
    };
}
