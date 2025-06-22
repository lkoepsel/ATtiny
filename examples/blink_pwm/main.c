// Quick and dirty demo of how to get PWM on any pin with interrupts
// ------- Preamble -------- //
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DELAY 10

volatile uint8_t brightnessA;
volatile uint8_t brightnessB;
volatile uint8_t TOP = 255;

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
    PORTB |= (_BV(PORTB1));
    OCR0A = brightnessA;
    OCR0B = brightnessB;
}
ISR(TIM0_COMPA_vect) 
{
    PORTB |= (_BV(PORTB1));
}
ISR(TIM0_COMPB_vect) 
{
    PORTB &= ~(_BV(PORTB1));
}

int main(void) {

    uint8_t i;
    DDRB |= (_BV(PORTB1));
    initTimer0();

    while (1) 
    {
        for (i = 0; i < TOP; i++) 
        {
            _delay_ms(DELAY);
            brightnessA = i;
        }
        _delay_ms(500);

        for (i = TOP; i > 0; i--) {
          _delay_ms(DELAY);
          brightnessA = i;
          // brightnessB = BOT - i;
        }
        _delay_ms(500);

  }                                                  // End event loop 
  return 0;                            // This line is never reached 
}
