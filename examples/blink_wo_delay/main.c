// Blink without Delay C version
// Based on Arduino tutorial:
// http://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
// And the Adafruit tutorial:
// https://learn.adafruit.com/multi-tasking-the-arduino-part-1?view=all

#include "sysclock.h"

#define interval 500

int main (void)
{
  // Variables will change:
  uint16_t previous_ticks = 0;        // will store last time LED was updated
  // Set up a system tick of 1 millisec (1kHz)
  init_sysclock_1k ();

    /* set pin to output*/
    DDRB |= (_BV(PORTB4));

    while(1)
    {
        // check to see if it's time to blink the LED; that is, if the 
        // difference between the current time and last time you blinked 
        // the LED is bigger than the interval at which you want to 
        // blink the LED.
        uint16_t current_ticks = ticks();

        if(current_ticks - previous_ticks > interval
           || previous_ticks > current_ticks) 
        {
        // save the last time you blinked the LED 
        previous_ticks = current_ticks;   
        // toggle the state of the LED
          PINB |= (_BV(PORTB4));
        }
    }
}
