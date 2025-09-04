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

// Define hardware, YELLOW/BLUE/YELLOW/BUTTON
#define YELLOW PB0
#define BLUE PB1
#define WHITE PB2
#define BUTTON PB4
#define LED_05 50
#define LED_10 100
#define LED_15 150
#define LED_20 200
#define LED 25 250

// ****Defined Interrupt Service Routines****
volatile uint8_t ticks_ctr = 0;

// Required for ticks timing, see examples/ticks
// Enabled by init_sysclock_1() in sysclock.c
ISR (TIM0_COMPA_vect)      
{
    ticks_ctr++;
}

// ****End of Defined Interrupt Service Routines****

// Initialize timer to 255 ticks for 2.55 seconds (1 tick = 10ms)
void init_sysclock_100 (void)          
{
    // Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
    // The values below will result in a 1kHz counter (1000 ticks = 1 second)
    // CTC mode (WGM0[2:0] = 2)
    // Set clock select to /8 CS => 010
    // Bit 2 – OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
    // COM0A0 - set to view OC0A on PB0 with scope
    // OCR0A = x9c or ~150

    // TCCR0A [ COM0A1 COM0A0 COM0B1 COM0B0 0 0 WGM01 WGM00 ] = 0b01000010
    // TCCR0B [ FOC0A FOC0B 0 0 WGM02 CS02 CS01 CS00 ] = 0b00000010
    // TIMSK0 [ 0 0 0 0  OCIE0B  OCIE0A  TOIE0 0 ] = 0b00000100
    // OCR0A = 0x9a
    // tick = 1/1000 second
    // Test using example/ticks w/ _delay_ms(1000); = 1000 ticks

    TCCR0A = ( _BV(COM0A0) | _BV(WGM01) ) ; 
    TCCR0B |= ( _BV(CS01) ) ;
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 0x79;
    sei();
 
    /* set pin to output to view OC0A*/
    DDRB |= (_BV(PORTB0));
}

// ****End of Defined Timer Setup Functions****

uint8_t ticks(void) {
    return(ticks_ctr);
}

int main (void)
{
    // Initialize timer to 255 ticks for 2.55 seconds (1 tick = 10ms)
    init_sysclock_100 ();

    // Blink all three LEDs to indicate game will start
    /* set pins to output */
    DDRB |=( _BV(BLUE) | _BV(WHITE) | _BV(YELLOW));  // PB0, PB1 and PB2 as outputs

    // set BUTTON to INPUT PULLUP (set to DDRD to INPUT then set PORTB)
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (BUTTON));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (BUTTON));

    // blink all LEDs to indicate start
    PORTB |=( _BV(BLUE) | _BV(WHITE) | _BV(YELLOW));
    _delay_ms(250);
    PORTB &= ~( _BV(BLUE) | _BV(WHITE) | _BV(YELLOW));  // set all low

    for (;;) 
    {
        uint8_t i = 5;
        do 
        {
                // Light YELLOW a LED_TIME between .5 and 2.5 seconds
                asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
                volatile uint8_t LED_TIME = 0;
                uint8_t j = i;
                do
                {
                    _delay_ms(LED_05);
                    LED_TIME += LED_05;
                } while (j--);

                asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
                _delay_ms(500);

                // Start timer
                // uint8_t start = ticks();
                
                // When button is pressed, determine PRESS_TIME
                static uint8_t button_state = 0;
                bool PRESSED = false;
                asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (WHITE));

                while (!PRESSED)
                {
                    // Shift previous states left and add current state
                    button_state = (button_state << 1) | (!(PINB & (1 << BUTTON))) | 0xE0;
                    // Button is pressed when last 5 readings are all low (pressed)
                    if (button_state == 0xF0) 
                    {
                        PRESSED = true;
                        asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (WHITE));
                        _delay_ms(50);
                    }
                }
                
                
                // Compare PRESS_TIME to LED_TIME
                
                
                // If CLOSE, blink BLUE else, blink YELLOW
                
                
                // Repeat 4 more times, with variable LED_TIME
                
        } while (--i);
        _delay_ms(1000);

        // Blink BLUE for every success
        
        
        // Blink YELLOW for every failure

    };
}
