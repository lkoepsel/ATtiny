// ticks - demonstrate time counter using a system clock
// Sets up a system tick of 1 millisec (1kHz)
// To test, uses the system delay (blocking, doesn't use clock)
// to determine delta between a delay
// There can be a lag of 1-2 milliseconds at times
// Requires init_sysclock()
// Runs ten times, printing the values to EEPROM
// When finished, use gdb x/10dh 0x810000 to see values
// Typically 1001/1002 for sysclock_1k setup

// while 1200000/8/0x96 = 1000, measurements see the following:
// using OCRA0 = 0x8f - toggles every 1ms based on scope
// using OCRA0 = 0x9a - _delay_ms(1000) measures 1001 ticks
 
#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "sysclock.h"

volatile uint16_t delta_ticks = 0;

int main (void)
{
    // init_sysclock_1k is required to initialize the counter for 1Khz ticks
    uint16_t eeprom_addr = 0;
    init_sysclock_1k ();

    for (uint8_t i = 9; i >= 0; i--)
    {
        uint16_t prior_ticks = ticks();
        _delay_ms(1000);
        delta_ticks = ticks() - prior_ticks;

        eeprom_update_word((uint16_t *)(eeprom_addr), delta_ticks);
        eeprom_addr += sizeof(eeprom_addr);

    }
    for (;;) {};
}
