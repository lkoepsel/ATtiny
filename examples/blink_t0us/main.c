// t0us - demonstrate timer counter 0
// need to confirm accuracy

#include <avr/io.h>
#include "ATtiny.h"

void timer0_delay_us(uint16_t us)
{
    // Timer clock with prescaler /1: 1.2MHz
    // Each timer tick = 0.833us
    
    // Calculate ticks (us * 6 / 5 approximates us / 0.833)
    uint16_t ticks = (us * 6) / 5;
    
    TCCR0B = (1 << CS00);  // Prescaler /1
    
    while (ticks > 0)
    {
        TCNT0 = 0;
        uint8_t wait = (ticks > 255) ? 255 : ticks;
        while (TCNT0 < wait) {} ;
        ticks -= wait;
    }
    
    TCCR0B = 0;  // Stop timer
}

int main (void)
{
    // set pin to output
    DDRB |= (_BV(PORTB4));

    for (;;)
    {
        SBI(PORTB, PORTB4);
        timer0_delay_us(1000);
        CBI(PORTB, PORTB4);
        timer0_delay_us(1000);

    }
}
