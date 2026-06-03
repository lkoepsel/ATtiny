//  blink_asm - uses bit setting by asm commands
//  For smallest code size, set LIBRARY = no_lib in env.make
//  Using the asm commands, ensures the specific method desired is used
//  In this case, sbi is used to set the bit in PORTB, cbi to clear
//  Testing has confirmed _BV doesn't always use SBI

#include <avr/io.h>
#include <util/delay_basic.h>

#define GREEN 0
#define delay 1200
int main(void)
{
    /* set pin to output*/
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (GREEN));

    for(;;)
    {
        /* turn led on and off */
        asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (GREEN));
        _delay_loop_2(delay);
    }
    return 0;
}
