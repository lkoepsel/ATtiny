//  blink_min - minimal code implementation of blink
//  Uses macros and link option -nostartfiles to remove interrupt vectors (38 bytes)
//  See line ~92 in MAKEFILE to set parameter
//  When using -nostartfiles, you also lose:
//      Stack pointer initialization
//      .data section initialization
//      .bss section clearing

#include <avr/io.h>
#include <util/delay.h>
#include "ATtiny.h"

// required if -nostartfiles is specified
int main(void) __attribute__((naked, section(".init9")));

int main(void)
{
    /* set pin to output*/
    SBI(DDRB, PORTB0);

    for(;;) 
    {
        /* turn led on and off */
        SBI(PINB, PORTB0);
        _delay_ms(500);
    }
    return 0; 
}
