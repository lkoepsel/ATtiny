// reaction - simple reaction time game
// Game Rules
1. Light YELLOW between .5 and 2 seconds
2. Blink GREEN to start.
3. User must press button to match the time of the YELLOW  
4. GREEN if acceptable
5. RED if not acceptable
6. Five chances, at the end, blink GREEN for successful turns and blink RED for unsuccessful

//
//                     ATtiny13
//                   ┌──────────┐
//     RESET/PB5 ──1─┤          ├─8── VCC
//           PB3 ──2─┤          ├─7── PB2/YELLOW
//    BUTTON/PB4 ──3─┤          ├─6── PB1/GREEN
//           GND ──4─┤          ├─5── PB0/RED
//                   └──────────┘
//

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

// Define hardware, RED/GREEN/YELLOW/BUTTON
#define RED PB0
#define GREEN PB1
#define YELLOW PB2
#define BUTTON PB4
#define LED_05
#define LED_10
#define LED_15
#define LED_20
#define LED 25

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

    for (;;) 
    {
        // Blink all three LEDs to indicate game will start
        /* set pins to output */
        DDRB |=( _BV(GREEN) | _BV(YELLOW) | _BV(RED));  // PB0, PB1 and PB2 as outputs
        // Make sure PB4 is an input (it should be by default)
        DDRB &= ~_BV(BUTTON);  // Clear DDB4 to ensure PB4 is input
        PORTB |=( _BV(GREEN) | _BV(YELLOW) | _BV(RED));  // set all high
        _delay_ms(500);
        PORTB &= ~( _BV(GREEN) | _BV(YELLOW) | _BV(RED));  // set all low

        for (uint8_t i=4; i==0; i--)
        {
                // Light YELLOW a LED_TIME between .5 and 2.5 seconds
                PORTB |=( _BV(YELLOW) );  // set YELLOW high
                uint8_t LED_TIME = 0;
                for (uint8_t j=i; j==0; j--)
                {
                    _delay_ms(LED_05);
                    LED_TIME += LED_05;
                }
                PORTB &= ~( _BV(YELLOW) );  // set YELLOW low

                // Start timer
                uint8_t start = ticks();
                
                // When button is pressed, determine PRESS_TIME
                
                
                // Compare PRESS_TIME to LED_TIME
                
                
                // If CLOSE, blink GREEN else, blink RED
                
                
                // Repeat 4 more times, with variable LED_TIME
                
        }

        // Blink GREEN for every success
        
        
        // Blink RED for every failure

    };
}
