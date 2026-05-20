// soft serial - software-defined serial port using assembly primitives
// Slow serial port, use for non-intensive serial interaction
// Change serial pins and timing in Library/registers.h and Library/softserial.S

#include <avr/pgmspace.h>
#include "softserial_asm.h"

#define CR 13
#define LF 10

const char prompt[]  PROGMEM = "13A";
const char waiting[] PROGMEM = "W:";

// Write a PROGMEM-resident, null-terminated string to the serial port.
static void pgmtext_write(const char *p)
{
    for (uint8_t c; (c = pgm_read_byte(p)); p++)
        char_write(c);
}

int main(void)
{
    init_serial();

    char_write(CR);
    char_write(LF);
    pgmtext_write(prompt);
    char_write(CR);
    char_write(LF);
    pgmtext_write(waiting);

    // Echo each received character back over the serial port.
    for (;;) {
        char_write(char_read());
    }
}
