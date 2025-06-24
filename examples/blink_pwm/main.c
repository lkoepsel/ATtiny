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
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
    OCR0A = brightnessA;
    OCR0B = brightnessB;
}
ISR(TIM0_COMPA_vect) 
{
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
}
ISR(TIM0_COMPB_vect) 
{
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
}

int main(void) {

    uint8_t i;
    DDRB |= (_BV(YELLOW));
    initTimer0();

    while (1) 
    {
        for (i = 0; i < TOP; i++) 
        {
            _delay_ms(DELAY);
            brightnessA = i;
        }
        cli();
        _delay_ms(END_DELAY);
        sei();

        for (i = TOP; i > 0; i--) {
          _delay_ms(DELAY);
          brightnessA = i;
        //   brightnessB = TOP - i;
        }
        cli();
        _delay_ms(END_DELAY);
        sei();
  }                                                  // End event loop 
  return 0;                            // This line is never reached 
}
