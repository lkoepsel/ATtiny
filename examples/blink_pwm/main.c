// Quick and dirty demo of how to get PWM on any pin with interrupts
// ------- Preamble -------- //
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DELAY 10
#define END_DELAY 1000
#define GREEN PB0
#define YELLOW PB1
#define BLUE PB2

volatile uint8_t YELLOW_bright;
volatile uint8_t GREEN_bright;

// -------- Functions --------- //
static inline void initTimer0(void) 
{
    TCCR0A = 0 ;                                        // Normal operation 
    TCCR0B |= ( _BV(CS01) | _BV(CS00)) ;                // /64
    TIMSK0 |= (_BV(OCIE0B) | _BV(OCIE0A) | _BV(TOIE0)); // turn on all interrupts
    sei();
}

ISR(TIM0_OVF_vect) 
{
    OCR0A = YELLOW_bright;
    OCR0B = GREEN_bright;
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
}
ISR(TIM0_COMPA_vect) 
{
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
}
ISR(TIM0_COMPB_vect) 
{
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
}

int main(void) {

    DDRB |= (_BV(YELLOW) | _BV(GREEN));
    initTimer0();

    while (1) 
    {
        YELLOW_bright = 127;
        GREEN_bright = 255;
    }                                                  // End event loop 
  return 0;                            // This line is never reached 
}
