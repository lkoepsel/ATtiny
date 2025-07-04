// multitasking_blocking - One line kernal for multitasking
// https://www.embedded.com/a-multitasking-kernel-in-one-line-of-code-almost/
// Designed to demonstrate the 13A performing different tasks
// with different delays, using a single function pointer array.

#include <avr/io.h>
#include <util/delay.h>

#define NTASKS 3

void zero (void) {
    /* toggle led on and off */
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (PINB0));
    _delay_ms(2000);
    return;
} 

void one (void) {
    /* toggle led on and off */
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (PINB1));
    _delay_ms(1000);
    return;
} 

void two (void) {
    /* toggle led on and off */
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (PINB2));
    _delay_ms(500);
    return;
} 

void (*tasklist[NTASKS])() = {zero, one, two};

int main(void)
{
    DDRB |= (_BV(PINB0) | _BV(PINB1) | _BV(PINB2));

    while (1)
    {
    for (uint8_t taskcount=0; taskcount < NTASKS; ++taskcount)
        {
            (*tasklist[taskcount])();

        }
    }
    return 0; 
}

