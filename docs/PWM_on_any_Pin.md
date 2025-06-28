# Make: AVR Programming - Mike Williams

## Example 10.1 Brute Force PWM

```c
/*
 * Quick and dirty PWM Demo
 */

// ------- Preamble -------- //
#include <avr/io.h>                        /* Defines pins, ports, etc */
#include <util/delay.h>                     /* Functions to waste time */
#include "pinDefines.h"

#define LED_DELAY 2

int main(void) {

  uint8_t brightness = 0;
  int8_t direction = 1;
  uint8_t i;

  // -------- Inits --------- //

  LED_DDR = 0xff;                                     /* Init all LEDs */

  // ------ Event loop ------ //
  while (1) {

    // PWM
    for (i = 0; i < 255; i++) {
      if (i < brightness) {
        LED_PORT = 0xff;                                    /* turn on */
      }
      else {
        LED_PORT = 0;                                      /* turn off */
      }
      _delay_us(LED_DELAY);
    }

    // Brighten and dim
    if (brightness == 0) {
      direction = 1;
    }
    if (brightness == 255) {
      direction = -1;
    }
    brightness += direction;

  }                                                  /* End event loop */
  return 0;                            /* This line is never reached */
}
```


## Example 10.3 PWM on Any Pin

[Code Site](https://github.com/hexagon5un/AVR-Programming/tree/master)

```c
// Quick and dirty demo of how to get PWM on any pin with interrupts
// ------- Preamble -------- //
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "pinDefines.h"

#define DELAY 3

volatile uint8_t brightnessA;
volatile uint8_t brightnessB;

// -------- Functions --------- //
static inline void initTimer0(void) {
                                 /* must be /64 or more for ISR timing */
  TCCR0B |= (1 << CS01) | (1 << CS00);
                                     /* both output compare interrupts */
  TIMSK0 |= ((1 << OCIE0A) | (1 << OCIE1B));
  TIMSK0 |= (1 << TOIE0);                 /* overflow interrupt enable */
  sei();
}

ISR(TIMER0_OVF_vect) {
  LED_PORT = 0xff;
  OCR0A = brightnessA;
  OCR0B = brightnessB;
}
ISR(TIMER0_COMPA_vect) {
  LED_PORT &= 0b11110000;                    /* turn off low four LEDs */
}
ISR(TIMER0_COMPB_vect) {
  LED_PORT &= 0b00001111;                   /* turn off high four LEDs */
}

int main(void) {
  // -------- Inits --------- //

  uint8_t i;
  LED_DDR = 0xff;
  initTimer0();

  // ------ Event loop ------ //
  while (1) {

    for (i = 0; i < 255; i++) {
      _delay_ms(DELAY);
      brightnessA = i;
      brightnessB = 255 - i;
    }

    for (i = 254; i > 0; i--) {
      _delay_ms(DELAY);
      brightnessA = i;
      brightnessB = 255 - i;
    }

  }                                                  /* End event loop */
  return 0;                            /* This line is never reached */
}
```

## PWM on Any Pin
So far we’ve seen two ways to implement PWM in our AVR code. One method implements PWM entirely in code by looping and directly setting the pins on and off using the CPU. The “normal” hardware PWM method works significantly faster, but only on six designated pins, two for each timer.

If we want to implement PWM on an arbitrary pin, there is a trick, but it’s a little bit
of hack. Instead of using the built-in pin-toggling function of the timer/counter, we’ll instead use the interrupts to trigger our own code, and turn on and off pins from within ISRs. We don’t have to tie up the CPU with counting and waiting, as we did in the brute-force PWM example. Rather, we can use a timer/counter in Normal mode to do the counting for us.

Then we trigger interrupts at the beginning of the cycle to turn the PWM pins on
and use the output-compare values to trigger another interrupt to turn the pins back off. So this method is a little bit like a hybrid of the brute force and fully hardware PWM methods: the counter and ISRs make it take less than the full CPU time just for the PWM, but because the ISRs take some CPU time, it’s not as fast or rock solid as fully hardware PWM.

Because we’re using ISRs to turn on and off the pins in question, we have to be a little bit careful that the PWM values are long enough that the ISRs have time to execute. Imagine that we set the PWM duty cycle to some small number like 6, and the counter’s CPU clock divider to its fastest mode. We then only have six CPU cycles to execute the ISR that turns the LED on at the beginning of the cycle, and this won’t work—most ISRs take at least 10 cycles just in program overhead. (We can further hack around this limitation, but at some point it’s not worth it.) So the trick to making this any-pin PWM work is making sure that we’ve set up the clock prescaler to at least divide by 64. Then we’ll have plenty of time for our in-
terrupts, and all is well.

### PWM on Any Pin Demo
To recap, the PWM-on-any-pin code works by setting up a timer/counter in Normal mode—counting up from 0 to 255 continuously—and interrupts are set to trigger off the timer. First, the overflow interrupt triggers when the timer rolls over back to 0. In this ISR, we turn the pins on. An output-compare ISR then turns the pin back off once the counter reaches the values stored in the output-compare register. That way, a larger value in the OCR registers mean that the pin is on for more of the cycle. 

There are also a couple of details to clean up. First, notice that there are two global variables defined, brightnessA and brightnessB, that will be used to load up the output-compare registers at the beginning of every cycle. Why not write to OCR0A and OCR0B directly? Depending on the timing of exactly when the variable is written and its value, the PWM can glitch for one cycle. A way around that is to always set the output-compare registers at a predictable point in the cycle—inside the over-flow ISR. These two global variables can be set by our code in main() whenever, but they’re only loaded into the OCR0 registers from inside the overflow ISR. It’s a simple buffer.

Triggering three different interrupts from one timing source is surprisingly easy. In the initTimer0 code, we can see that it basically amounts to setting three different bits in the TIMSK (timer interrupt mask) register. As mentioned previously, the clock source for the timer has to be significantly slower than the CPU clock to allow the ISRs time to run, so I’ve use a divide-by-64 clock setting. Finally, as with all situations when you’re using an interrupt, don’t forget to enable the global interrupt bit using
the sei() function.

The rest of the code is straightforward. The ISRs should be kept as short as possible because they’re being called quite frequently. Here, the compare interrupts have only one task. Finally, the main() function demonstrates how to use the any-pin PWM functionality. We simply set the global variable to the desired level whenever it suits us, and the timer and interrupts take care of the rest.