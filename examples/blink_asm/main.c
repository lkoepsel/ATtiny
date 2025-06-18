//  blink_asm - uses bit setting by asm commands
//  For smallest code size, set LIBRARY = no_lib in env.make 
//  Using the asm commands, ensures the specific method desired is used
//  In this case, sbi is used to set the bit in PINB, toggling its value
//  Testing has confirmed _BV doesn't always use SBI

#include <avr/io.h>
#include <util/delay.h>
 
int main(void)
{
    /* set pin to output*/
    // DDRB |= (_BV(PINB4));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (DDB4));

    while(1) 
    {
        /* turn led on and off */
        // PINB |= (_BV(PORTB0));
        asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (PINB4));
        _delay_ms(1000);
    }
    return 0; 
}
