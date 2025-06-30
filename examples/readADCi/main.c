// readADCi - reads the ADC using interrupts
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// -------- Functions --------- //
// ADC variables
volatile uint16_t ADC_result = 0;

// ADC interrupt - just store the result
ISR(ADC_vect)      
{
    ADC_result = ADC;
}

// Thread-safe ADC result reading
uint16_t read_ADC(void) 
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        return ADC_result;
    }
    return 0;   
}

// ADC initialization for potentiometer on PB2 (ADC1)
static inline void initADC(void) 
{
    // Select ADC1 (PB2), VCC as reference
    ADMUX = _BV(MUX0);  // Select ADC1 (PB2)
    
    // Enable ADC with prescaler /16, enable interrupts and auto-trigger
    ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADATE) | _BV(ADPS2);
    sei();
}

int main(void)
{
    initADC0();

    /* set pins to output */
    DDRB |= _BV(DDB4) | _BV(DDB3);  // PB4 and PB3 as outputs
    // Make sure PB2 is an input (it should be by default)
    DDRB &= ~_BV(DDB2);  // Clear DDB2 to ensure PB2 is input
    PORTB |= _BV(PB4);  // Set PB4 high initially

    ADCSRA |= _BV(ADSC);                    // start initial ADC conversion

    for (;;)
    {
        uint16_t result = read_ADC();                  // read ADC value

        // For debugging: toggle LED based on threshold
        if (result > 512) {
            PORTB |= _BV(PB3);   // LED on
        } else {
            PORTB &= ~_BV(PB3);  // LED off
        }

        _delay_ms(100);  // Add delay between readings
    }
}
