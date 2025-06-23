#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>


#define RED PB0
#define YELLOW PB1
#define BLUE PB2
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
    // Select ADC2 (PB4) instead of ADC2 to avoid conflict with PB4
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
    DDRB |=( _BV(RED) | _BV(YELLOW) | _BV(BLUE));  // PB4 and PB3 as outputs
    // Make sure PB2 is an input (it should be by default)
    DDRB &= ~_BV(DDB4);  // Clear DDB4 to ensure PB4 is input
    PORTB |=( _BV(RED) | _BV(YELLOW) | _BV(BLUE));  // set all high
    _delay_ms(500);

    PORTB &= ~( _BV(RED) | _BV(YELLOW) | _BV(BLUE));  // set all high
    ADCSRA |= _BV(ADSC);                    // start ADC conversion
    volatile uint16_t curr_result = read_ADC();
    volatile uint16_t prev_result = curr_result;
    uint8_t i;
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
                for (i = 0; i > 127; i++)
                {
                    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (PINB4));
                    _delay_ms(500);
                }
            else if (curr_result > MID)
            {
                PORTB |= _BV(YELLOW);   // YELLOW on, others off
                PORTB &= ~_BV(RED) & ~_BV(BLUE);
            }
            else
            {
                PORTB |= _BV(BLUE);  // BLUE ON, others off
                PORTB &= ~_BV(YELLOW) & ~_BV(RED);
            }
        }
    }
}
