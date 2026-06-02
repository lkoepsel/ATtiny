// serial_asm.h
// C declarations for the assembly routines in serial.S
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialise TX/RX pin directions and idle state.
// Must be called before char_write or char_read.
void init_sysclock_1k(void);

// Transmit one byte at 9600-8-N-1.
// The character is passed in r24 per the AVR-GCC ABI.
uint16_t ticks(void);

#ifdef __cplusplus
}
#endif
