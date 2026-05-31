; =============================================================
; flash_w  -  sequential FLASH write for ATtiny13A
; Target : ATtiny13A (64 bytes EEPROM, 6-bit address)
; Toolchain: avr-gcc (preprocessor + assembler)
; =============================================================
;
; The "flash address" needs to be in the Z-register r31/r30.
;
; Calling convention: AVR-GCC ABI (r31/r30 FLASH address, r24 out, r18 scratch).

#include <avr/io.h>

.section .text

; void flash_write(uint8_t addr)
;   Set the Z address to PROGMEM text on entry

; .global flash_write
; flash_write:
;         lpm     r24, Z+     ; load byte at Z into r24, the char_write register
;         tst     r24          ; exit if null byte
;         breq    print_done
;         rcall   char_write  ; write char to console
;         rjmp    flash_write

; print_done:
;     ret
