// Quick and dirty demo of how to get PWM on any pin with interrupts
// ------- Preamble -------- //
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#define GREEN PB0
#define YELLOW PB1

volatile uint8_t A_pulse = 32;
volatile uint8_t B_pulse = 63;

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
    OCR0A = A_pulse;
    OCR0B = B_pulse;
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
}
ISR(TIM0_COMPA_vect) 
{
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
}
ISR(TIM0_COMPB_vect) 
{
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
}

int main(void) {

    DDRB |= (_BV(YELLOW) | _BV(GREEN));
    initTimer0();

    while (1) 
    {
        _NOP();
    }                                         
  return 0;                            // This line is never reached 
}
