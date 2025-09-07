// ATtiny: Local file of macros and defines for ATtiny13A
// Typically, included in same folder as main, to keep code size small

#ifndef ATTINY_H
#define ATTINY_H

#include <stdlib.h>
#include <avr/io.h>

#define SBI(port, bit) \
    __asm__ __volatile__ ( \
        "sbi %0, %1" \
        : \
        : "I" (_SFR_IO_ADDR(port)), \
          "I" (bit) \
    )

    #define CBI(port, bit) \
    __asm__ __volatile__ ( \
        "cbi %0, %1" \
        : \
        : "I" (_SFR_IO_ADDR(port)), \
          "I" (bit) \
    )

#endif
