; =============================================================
; softserial  –  serial program and library
; Target : ATtiny13A (1.2 MHz default internal RC oscillator)
; Toolchain: avr-as / avr-ld  (GNU Binutils for AVR)
; =============================================================

#include <avr/io.h>
#include "registers.h"

; ====================================================================
;  TEXT SECTION  (executable code lives here)
; ====================================================================
.section .text

; ---------- Registers and Values ----------------
; r24                           ; char register
; r18                           ; temp register
; r19                           ; timer delay register
; r20                           ; temp register

#define    period       37      ; # of ticks for 1 bit period (9600 baud @ 1.2MHz)
#define    half_period  20      ; # of ticks for  a .5 bit period
#define    TRIM         0x60    ; OSCCAL trim value, use examples/osccal to determine

; ====================================================================
;  Subroutines SECTION
; ====================================================================

;initialize serial port, must be called before using char_read and char_write
.global init_serial
init_serial:

    ldi     r18, TRIM           ; osc trim value
    out     RCCAL, r18          ; apply measured OSCCAL trim (via examples/osccal)

;   Set TX pin as output, set RX pin as input pullup
    sbi     IO_DDR, TX
    cbi     IO_DDR, RX
    sbi     IO_PORT, RX

    ; set TX high to start, start_bit is low
    sbi     IO_PORT, TX
    ret
; --------------------------------------------------------------------

; write a char (passed in r24, per AVR-GCC ABI) to the serial port
.global char_write
char_write:

    ; Start bit
    cbi     IO_PORT, TX
    delay_ticks period

    ;  8 data bits and preserve char
    ldi     r20, 8
    mov     r18, r24

write_bit:
    ror     r18
    brcs    write_one
    cbi     IO_PORT, TX
    rjmp     next_write

write_one:
    sbi     IO_PORT, TX

next_write:
    delay_ticks period

    dec     r20
    brne    write_bit

    ;  Stop bit
    sbi     IO_PORT, TX
    delay_ticks period
    ret
; --------------------------------------------------------------------


; char_read - receive one char into r24 (8N1, LSB first), per AVR-GCC ABI
.global char_read
char_read:
;   Wait for start bit: idle is HIGH, start bit is LOW
;   while (IO_PIN & (1 << RX)) {} ;
wait_start:
    in      r20, IO_PIN
    sbrc    r20, RX    ; skip rjmp when RX is LOW = start bit
    rjmp    wait_start

;   Wait a .5 bit period so bit0 is sampled mid-bit
    delay_ticks half_period

    in      r20, IO_PIN
    sbrc    r20, RX    ; confirm start bit remains low
    rjmp    wait_start

;   Wait a 1 bit period for a total of 1.5 bit periods
    delay_ticks period

;   Read 8 data bits, LSB first, into r24
    ldi     r20, 8              ; bit counter

read_bit:
    in      r18, IO_PIN           ; scratch read - do not clobber r24
    clc                         ; assume bit is 0
    sbrc    r18, RX         ; skip sec when RX is LOW
    sec                         ; RX HIGH -> bit is 1
    ror     r24                 ; shift carry into MSB (LSB-first)

    delay_ticks period

    dec     r20
    brne    read_bit

    ret
; --------------------------------------------------------------------


; ====================================================================
;  DATA SECTION  (initialized variables in SRAM)
;  Declare with:  my_var: .byte 0
; ====================================================================
.section .data

; ====================================================================
;  BSS SECTION  (zero-initialized / uninitialized variables in SRAM)
;  Declare with:  my_buf: .skip 16
; ====================================================================
.section .bss
