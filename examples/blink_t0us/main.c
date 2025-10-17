// t0us - demonstrate timer counter 0
// need to confirm accuracy
// Values below provide a 104.4us time period

#include <avr/io.h>
#include "ATtiny.h"
#define baud_ticks 156 // 1200 baud

int main (void)
{
    // set pin to output
    DDRB |= (_BV(PORTB0));
    TCCR0B = (1 << CS01);  // Prescaler /8
    OSCCAL = 0x73;

    do
    {
        
        SBI(PORTB, PORTB0);
        TCNT0 = 0;
        while (TCNT0 < baud_ticks) {} ;
        CBI(PORTB, PORTB0);
        TCNT0 = 0;
        while (TCNT0 < baud_ticks) {} ;
    } while (1);
}
