#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>


#define GREEN PB0
#define YELLOW PB1
#define BLUE PB2
#define POT PB4
#define TOP 642
#define MID 341

volatile uint16_t ADC_result = 0;

// -------- Functions --------- //
ISR (ADC_vect)      
{
    ADC_result = ADC;                  // read ADC value
}

uint16_t read_ADC(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        return(ADC_result);
    }
    return 0;   
}

static inline void initADC0(void) 
{
    ADMUX = 0;  // Clear ADMUX first
    ADMUX |= _BV(MUX1);  // Select ADC2 (PB4), REFS0 = 0, VCC as reference

    // Enable ADC with prescaler /16 (for 9.6MHz clock → 600kHz ADC clock)
    // Enable ADC interrupts
    ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADATE) | _BV(ADPS2);
    sei();
}

int main(void)
{
    initADC0();

    /* set pins to output */
    DDRB |=( _BV(GREEN) | _BV(YELLOW) | _BV(BLUE)); 
    // Make sure PB2 is an input (it should be by default)
    DDRB &= ~_BV(POT);
    PORTB |=( _BV(GREEN) | _BV(YELLOW) | _BV(BLUE));  // set all high
    _delay_ms(500);

    PORTB &= ~( _BV(GREEN) | _BV(YELLOW) | _BV(BLUE));  // set all high
    ADCSRA |= _BV(ADSC);                    // start ADC conversion

    volatile uint16_t curr_result = read_ADC();
    volatile uint16_t prev_result = curr_result;
    for (;;)
    {
        curr_result = read_ADC();
        prev_result = curr_result;

        while (prev_result == curr_result)
        {
            curr_result = read_ADC();
            if (curr_result > TOP) 
            {
                PORTB &= ~_BV(YELLOW) & ~_BV(BLUE);
                PORTB |= _BV(GREEN);  
            }
            else if (curr_result > MID)
            {
                PORTB |= _BV(YELLOW);
                PORTB &= ~_BV(GREEN) & ~_BV(BLUE);
            }
            else
            {
                PORTB |= _BV(BLUE);
                PORTB &= ~_BV(YELLOW) & ~_BV(GREEN);
            }
        }
    }
}
