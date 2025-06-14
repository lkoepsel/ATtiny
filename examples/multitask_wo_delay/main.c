// One line kernal for multitasking w/o delay

#include <avr/io.h>
#include "sysclock.h"


#define yellow_interval 500
#define white_interval 500
#define red_interval 500

int main(void)
{
  uint32_t yellow_ticks= 0;        // will store last time LED was updated
  uint32_t white_ticks= 0;        // will store last time LED was updated
  // uint32_t red_ticks= 0;        // will store last time LED was updated

  // Set up a system tick of 1 millisec (1kHz)
  init_sysclock_1k ();

  DDRB |= (_BV(PINB0) | _BV(PINB1) | _BV(PINB4));

    while(1)
    {
        // check to see if it's time to blink the LED; that is, if the 
        // difference between the current time and last time you blinked 
        // the LED is bigger than the interval at which you want to 
        // blink the LED.
        uint32_t current_ticks = ticks();

        if(current_ticks - yellow_ticks > yellow_interval) 
        {
        // save the last time you blinked the LED 
        yellow_ticks = current_ticks;   
        // toggle the state of the LED
          PINB |= (_BV(PORTB0));
        }

        if(current_ticks - white_ticks > white_interval) 
        {
        // save the last time you blinked the LED 
        white_ticks = current_ticks;   
        // toggle the state of the LED
          PINB |= (_BV(PORTB1));
        }

        // if(current_ticks - red_ticks> red_interval) 
        // {
        // // save the last time you blinked the LED 
        // red_ticks= current_ticks;   
        // // toggle the state of the LED
        //   PINB |= (_BV(PORTB4));
        // }

    }

}

