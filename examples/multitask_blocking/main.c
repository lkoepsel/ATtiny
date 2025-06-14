// One line kernal for multitasking
// https://www.embedded.com/a-multitasking-kernel-in-one-line-of-code-almost/
// Designed to demonstrate the 13A performing different tasks
// with different delays, using a single function pointer array.

#include <avr/io.h>
#include <util/delay.h>

#define NTASKS 3

void zero (void) {
    /* toggle led on and off */
    PINB |= _BV(PINB0);
    _delay_ms(125);
    return;
} 

void one (void) {
    /* toggle led on and off */
    PINB |= _BV(PINB1);
    _delay_ms(125);
    return;
} 

void four (void) {
    /* toggle led on and off */
    PINB |= _BV(PINB4);
    _delay_ms(250);
    return;
} 

void (*tasklist[NTASKS])() = {zero, one, four};

int main(void)
{
    DDRB |= (_BV(PINB0) | _BV(PINB1) | _BV(PINB4));

    while (1)
    {
    for (uint8_t taskcount=0; taskcount < NTASKS; ++taskcount)
        {
            (*tasklist[taskcount])();

        }
    }
    return 0; 
}

