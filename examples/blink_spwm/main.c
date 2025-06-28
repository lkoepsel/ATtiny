//  blink_spwm - uses bit setting by asm commands, and creates a soft PWM
//  based on Mike Williams Brute Force PWM routine 10.1

// ------- Preamble -------- //
#include <avr/io.h>                        /* Defines pins, ports, etc */
#include <util/delay.h>                     /* Functions to waste time */

#define LED_DELAY 2
#define PEAK_DELAY 1000
#define GREEN 0
#define BRIGHT 100
#define DIM 0

int main(void) {

  uint8_t brightness = 0;
  int8_t direction = 1;
  uint8_t i;

  // -------- Inits --------- //

    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (GREEN));

  // ------ Event loop ------ //
  while (1) 
  {
    // PWM
    for (i = DIM; i < BRIGHT; i++) 
    {
      if (i < brightness) 
      {
        asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));                                    /* turn on */
      }
      else 
      {
        asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));                                      /* turn off */
      }
      _delay_us(LED_DELAY);
    }

    // Brighten and dim
    if (brightness == DIM) 
    {
        direction = 1;
        _delay_ms(PEAK_DELAY);
    }
    if (brightness == BRIGHT) 
    {
        direction = -1;
        _delay_ms(PEAK_DELAY);

    }
    brightness += direction;
  }                                                  /* End event loop */
  return 0;                            /* This line is never reached */
}
