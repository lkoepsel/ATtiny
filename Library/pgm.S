; =============================================================
; pgm  -  sequential program-memory (flash) read for ATtiny13A
; Target : ATtiny13A (1 KB flash; 16-bit byte addresses 0..0x3FF)
; Toolchain: avr-gcc (preprocessor + assembler)
; =============================================================
;
; Mirrors the eeprom.S pair (init + read_next), but flash has no
; persistent address SFR like EEAR, so the 16-bit pointer lives in
; 2 bytes of SRAM. Reading stops on a null byte is the caller's
; job - the routine just returns the byte and post-increments.
;
; Calling convention: AVR-GCC ABI.
;   addr in r25:r24   (low byte r24, high byte r25)
;   byte out in r24
;   r30/r31 (Z) used as scratch (call-clobbered, no save needed)

#include <avr/io.h>

.section .bss
pgm_cursor: .skip 2     ; 16-bit current flash byte address

.section .text

; void pgm_read_init(const void *addr)
;   Set the flash read cursor to `addr`. Pass the address of any
;   PROGMEM byte (typically a string in const char[] PROGMEM).
.global pgm_read_init
pgm_read_init:
    sts     pgm_cursor,   r24    ; low byte
    sts     pgm_cursor+1, r25    ; high byte
    ret

; uint8_t pgm_read_next(void)
;   Return the byte at the current cursor, then advance by one.
;   `lpm Rd, Z+` reads via the Z pointer and post-increments Z in
;   a single instruction.
.global pgm_read_next
pgm_read_next:
    lds     r30, pgm_cursor      ; Z low  <- cursor low
    lds     r31, pgm_cursor+1    ; Z high <- cursor high
    lpm     r24, Z+              ; byte -> r24, Z++
    sts     pgm_cursor,   r30    ; cursor low  <- Z low
    sts     pgm_cursor+1, r31    ; cursor high <- Z high
    ret
