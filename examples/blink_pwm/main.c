// Quick and dirty demo of how to get PWM on any pin with interrupts
// ------- Preamble -------- //
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DELAY 5

volatile uint8_t brightnessA;
volatile uint8_t brightnessB;

// -------- Functions --------- //
static inline void initTimer0(void) 
{
    TCCR0A = 0 ;                // Normal operation 
    TCCR0B |= ( _BV(CS01) | _BV(CS00)) ; // /64
    TIMSK0 |= (_BV(OCIE0B) | _BV(OCIE0A) | _BV(TOIE0)); // turn on all interrupts
    sei();
}

ISR(TIM0_OVF_vect) 
{
    PORTB |= (_BV(PORTB1) | _BV(PORTB0));
    OCR0A = brightnessA;
    OCR0B = brightnessB;
}
ISR(TIM0_COMPA_vect) 
{
    PORTB &= ~(_BV(PORTB0));
    PORTB |= (_BV(PORTB1));
}
ISR(TIM0_COMPB_vect) 
{
    PORTB |= (_BV(PORTB0));
    PORTB &= ~(_BV(PORTB1));
}

int main(void) {

    uint8_t i;
    DDRB |= (_BV(PORTB0) | _BV(PORTB1));
    initTimer0();

    while (1) {

    // loop to fade bright (fade on)
    for (i = 0; i < 255; i++) 
    {
        _delay_ms(DELAY);
        brightnessA = i;
        brightnessB = 255 - i;
    }
    _delay_ms(1000);

    // loop to fade dim (fade off)
    for (i = 254; i > 0; i--) {
      _delay_ms(DELAY);
      brightnessA = i;
      brightnessB = 255 - i;
    }
    _delay_ms(1000);

  }                                                  // End event loop 
  return 0;                            // This line is never reached 
}
