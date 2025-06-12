// millis - demonstrate time counter using a system clock
// Sets up a system tick of 1 millisec (1kHz)
// To test, uses the system delay (blocking, doesn't use clock)
// to determine delta between a delay
// There can be a lag of 1-2 milliseconds at times
// Requires init_sysclock()
// Runs once. When finished, use gdb disp to check delta_ticks
#include <avr/io.h>
#include <util/delay.h>
#include "sysclock.h"

int main (void)
{
    // init_sysclock_1k is required to initialize the counter for 1Khz ticks
    // init_sysclock_100 is required to initialize the counter for 100hz ticks
    // pick 1
    init_sysclock_100 ();
    // init_sysclock_100 ();

    /* set pin to output*/
    DDRB |= (_BV(PORTB4));

    for (;;)
    {
    uint8_t prior_ticks = ticks();
    _delay_ms(1000);
    volatile uint8_t delta_ticks = ticks() - prior_ticks;
    // The statement below ensures delta_ticks isn't optimized away
    // and a breakpoint opportunity for gdb
    asm volatile("" : : "r" (delta_ticks) : "memory");
    }
}
