// One line kernal for multitasking w/o delay
// https://www.embedded.com/a-multitasking-kernel-in-one-line-of-code-almost/
// Designed to demonstrate the 13A performing different tasks
// with different delays, using a single function pointer array.

#include <avr/io.h>
#include <util/delay.h>

#define NTASKS 5

void zero (void) {
    /* toggle led on and off */
    PINB |= _BV(PINB0);
    _delay_ms(255);
    return;
} 

void one (void) {
    /* toggle led on and off */
    PINB |= _BV(PINB1);
    _delay_ms(100);
    return;
} 

void two (void) {
    /* toggle led on and off */
    PINB |= _BV(PINB2);
    _delay_ms(150);
    return;
} 

void three (void) {
    /* toggle led on and off */
    PINB |= _BV(PINB3);
    _delay_ms(200);
    return;
} 

void four (void) {
    /* toggle led on and off */
    PINB |= _BV(PINB4);
    _delay_ms(50);
    return;
} 

void (*tasklist[NTASKS])() = {zero, one, two, three, four};

int main(void)
{
    DDRB |= (_BV(PINB0) | _BV(PINB1) | _BV(PINB2) | _BV(PINB3) | _BV(PINB4));

    while (1)
    {
    for (uint8_t taskcount=0; taskcount < NTASKS; ++taskcount)
        {
            (*tasklist[taskcount])();

        }
    }
    return 0; 
}

