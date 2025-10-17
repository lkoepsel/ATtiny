// t0us - demonstrate timer counter 0
// need to confirm accuracy
// Values below provide a 104.4us time period

#include <avr/io.h>
#include "ATtiny.h"
#define baud_ticks 124 // 9600 baud

int main (void)
{
    // set pin to output
    DDRB |= (_BV(PORTB0));
    TCCR0B = (1 << CS00);  // Prescaler /1
    OSCCAL = 0x68;

    for (;;)
    {
        
        SBI(PORTB, PORTB0);
        TCNT0 = 0;
        while (TCNT0 < baud_ticks) {} ;
        CBI(PORTB, PORTB0);
        TCNT0 = 0;
        while (TCNT0 < baud_ticks) {} ;
    }
}
