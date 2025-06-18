// Blink without Delay C version
// Based on Arduino tutorial:
// http://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
// And the Adafruit tutorial:
// https://learn.adafruit.com/multi-tasking-the-arduino-part-1?view=all
// Uses asm commands, instead of C, as C doesn't use SBI in _BV

#include "sysclock.h"

#define interval 100
#define YELLOW 0        // yellow LED to pin 0
#define WHITE 3         // white LED to pin 3
#define RED 4           // red LED to pin 4

volatile uint16_t total_toggles = 0;
volatile uint16_t total_ro = 0;

int main (void)
{
  // Variables will change:
  uint16_t previous_ticks = 0;        // will store last time LED was updated
  // Set up a system tick of 1 millisec (1kHz)
  init_sysclock_1k ();

    /* set pin to output*/
    DDRB |= (_BV(YELLOW) | _BV(WHITE) | _BV(RED));
    PORTB |= (_BV(RED));
    PORTB |= (_BV(WHITE));

    while(1)
    {
        // check to see if it's time to blink the LED; that is, if the 
        // difference between the current time and last time you blinked 
        // the LED is bigger than the interval at which you want to 
        // blink the LED.
        uint16_t current_ticks = ticks();
        if (current_ticks < previous_ticks)
        {
            asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (RED));
            total_toggles++;
        }
        if(current_ticks - previous_ticks > interval ) 
        {
            // save the last time you blinked the LED 
            previous_ticks = current_ticks;   
            // toggle the state of the LED
            // PINB |= (_BV(YELLOW));
            asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (YELLOW));
            total_ro++;
        }
    }
}
