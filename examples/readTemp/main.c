#include <avr/io.h>
#include <util/delay.h>

// -------- Functions --------- //
static inline void initADC0(void) 
{
    // Select ADC1 (PB2) instead of ADC2 to avoid conflict with PB4
    ADMUX = 0;  // Clear ADMUX first
    ADMUX |= _BV(MUX0);  // Select ADC1 (PB2), REFS0 = 0, VCC as reference

    // Enable ADC with prescaler /16 (for 9.6MHz clock → 600kHz ADC clock)
    ADCSRA = _BV(ADEN) | _BV(ADPS2);
}

int main(void)
{
    initADC0();

    /* set pins to output */
    DDRB |= _BV(DDB4) | _BV(DDB3);  // PB4 and PB3 as outputs
    // Make sure PB2 is an input (it should be by default)
    DDRB &= ~_BV(DDB2);  // Clear DDB2 to ensure PB2 is input
    PORTB |= _BV(PB4);  // Set PB4 high initially

    for (;;)
    {
        ADCSRA |= _BV(ADSC);                    // start ADC conversion
        loop_until_bit_is_clear(ADCSRA, ADSC);  // wait until done
        uint16_t result = ADC;                  // read ADC value

        // For debugging: toggle LED based on threshold
        if (result > 512) {
            PORTB |= _BV(PB3);   // LED on
        } else {
            PORTB &= ~_BV(PB3);  // LED off
        }

        _delay_ms(100);  // Add delay between readings
    }
}
