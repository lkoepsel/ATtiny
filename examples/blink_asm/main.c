//  blink_asm - uses bit setting by asm commands
//  For smallest code size, set LIBRARY = no_lib in env.make 
//  Using the asm commands, ensures the specific method desired is used
//  In this case, sbi is used to set the bit in PORTB, cbi to clear
//  Testing has confirmed _BV doesn't always use SBI

#include <avr/io.h>
#include <util/delay.h>

#define GREEN 0
int main(void)
{
    /* set pin to output*/
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (GREEN));

    while(1) 
    {
        /* turn led on and off */
        asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
        _delay_ms(50);
        asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
        _delay_ms(50);
    }
    return 0; 
}
