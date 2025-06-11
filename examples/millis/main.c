// millis - demonstrate time counter using a system clock
// Sets up a system tick of 1 millisec (1kHz)
// To test, uses the system delay (blocking, doesn't use clock)
// to determine delta between a delay
// There can be a lag of 1-2 milliseconds at times
// Requires init_sysclock()
// Runs once. When finished, use gdb disp to check delta_ticks
#include <avr/io.h>
#include "delay.h"
#include "sysclock.h"

int main (void)
{
    // init_sysclock_2 is required to initialize the counter for millis()
    init_sysclock_0 ();

    for (;;)
    {
    uint16_t prior_ticks = ticks_ro();
    _delay_ms(1000);
    volatile uint16_t delta_ticks = ticks_ro() - prior_ticks;
    // At the end of your function, before return
    asm volatile("" : : "r" (delta_ticks) : "memory");
    }
}
