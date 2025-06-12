// One line kernal for multitasking w/o delay

#include <avr/io.h>
#include "sysclock.h"


#define interval_0 750
#define interval_1 500
#define interval_4 250

int main(void)
{
  uint32_t previous_ticks_0 = 0;        // will store last time LED was updated
  uint32_t previous_ticks_1 = 0;        // will store last time LED was updated
  uint32_t previous_ticks_4 = 0;        // will store last time LED was updated

  // Set up a system tick of 1 millisec (1kHz)
  init_sysclock_1k ();

//   DDRB |= (_BV(PINB0) | _BV(PINB1) | _BV(PINB2) | _BV(PINB3) | _BV(PINB4));
  DDRB |= (_BV(PINB0) );

    while(1)
    {
        // check to see if it's time to blink the LED; that is, if the 
        // difference between the current time and last time you blinked 
        // the LED is bigger than the interval at which you want to 
        // blink the LED.
        uint32_t current_ticks = ticks();

        if(current_ticks - previous_ticks_0 > interval_0
           || previous_ticks_0 > current_ticks) 
        {
        // save the last time you blinked the LED 
        previous_ticks_0 = current_ticks;   
        // toggle the state of the LED
          PINB |= (_BV(PORTB0));
        }

        if(current_ticks - previous_ticks_1 > interval_1
           || previous_ticks_1 > current_ticks) 
        {
        // save the last time you blinked the LED 
        previous_ticks_1 = current_ticks;   
        // toggle the state of the LED
          PINB |= (_BV(PORTB1));
        }

        if(current_ticks - previous_ticks_4 > interval_4
           || previous_ticks_4 > current_ticks) 
        {
        // save the last time you blinked the LED 
        previous_ticks_4 = current_ticks;   
        // toggle the state of the LED
          PINB |= (_BV(PORTB4));
        }

    }

}

