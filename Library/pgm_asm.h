// pgm_asm.h
// C declarations for the assembly routines in pgm.S
//
// Sequential program-memory (flash) read for ATtiny13A. Pair with the
// EEPROM routines in eeprom_asm.h - same API shape, different memory.
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Set the flash read cursor. Pass the address of a PROGMEM byte; the low
// 10 bits address the 1 KB of flash on ATtiny13A. The pointer is passed
// in r25:r24 per the AVR-GCC ABI.
void pgm_read_init(const void *addr);

// Return the byte at the current cursor and post-increment. The caller
// decides when to stop (e.g. on a null byte for C strings).
uint8_t pgm_read_next(void);

#ifdef __cplusplus
}
#endif
