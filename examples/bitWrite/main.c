// soft serial - software-defined serial port using assembly primitives
// Change serial pins and timing in Library/registers.S and Library/serial.S

#include <avr/pgmspace.h>
#include "serial_asm.h"

#define CR 13
#define LF 10

const char prompt[]  PROGMEM = "A";
const char waiting[] PROGMEM = "?";

// Write a PROGMEM-resident, null-terminated string to the serial port.
static void pgmtext_write(const char *p)
{
    for (uint8_t c; (c = pgm_read_byte(p)); p++)
        char_write(c);
}

int main(void)
{
    init_serial();
    /* set pin to output*/
    DDRB |= (_BV(PORTB2) | _BV(PORTB1) | _BV(PORTB0));

    char_write(CR);
    char_write(LF);
    pgmtext_write(prompt);
    char_write(CR);
    char_write(LF);
    pgmtext_write(waiting);

    // Echo each received character back over the serial port.
    for (;;) {
        uint8_t bits = char_read();
        if (bits <= 7)
        {
            PORTB = bits;
        }
        else
        {
            pgmtext_write(waiting);        
        }
    }
}
