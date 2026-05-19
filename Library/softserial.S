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
; r17                           ; char register
; r18                           ; temp register
; r19                           ; timer delay register
; r20                           ; temp register

#define    period       35      ; # of ticks for 1 bit period (9600 baud @ 1.2MHz)
#define    half_period  18      ; # of ticks for  a .5 bit period

; ====================================================================
;  Subroutines SECTION
; ====================================================================
; blocking delay, enter with r19 as 8-bit counter value
timer_delay:
    dec    r19
    brne   timer_delay
    ret
; --------------------------------------------------------------------

;initialize serial port, must be called before using char_read and char_write
init_serial:
;   Set TX pin as output, set RX pin as input pullup
    sbi     IO_DDR, TX
    cbi     IO_DDR, RX
    sbi     IO_PORT, RX

    ; set TX high to start, start_bit is low
    sbi     IO_PORT, TX
    ret
; --------------------------------------------------------------------

; write a char in r17 to serial port, r17 is preserved
char_write:

    ; Start bit
    cbi     IO_PORT, TX
    ldi     r19,period
    rcall   timer_delay


    ;  8 data bits and preserve char
    ldi     r20, 8
    mov     r18, r17

write_bit:
    ror     r18
    brcs    write_one
    cbi     IO_PORT, TX
    rjmp     next_write

write_one:
    sbi     IO_PORT, TX

next_write:
    ldi     r19,period
    rcall   timer_delay

    dec     r20
    brne    write_bit

    ;  Stop bit
    sbi     IO_PORT, TX
    ldi     r19,period
    rcall   timer_delay
    ret
; --------------------------------------------------------------------


; char_read - receive one char into r17 (8N1, LSB first)
;   Returns status in T flag:  T=0 frame OK,  T=1 framing error.
char_read:
;   Wait for start bit: idle is HIGH, start bit is LOW
;   while (IO_PIN & (1 << RX)) {} ;
wait_start:
    in      r20, IO_PIN
    sbrc    r20, RX    ; skip rjmp when RX is LOW = start bit
    rjmp    wait_start

;   Wait a .5 bit period so bit0 is sampled mid-bit
    ldi     r19, half_period
    rcall   timer_delay

    in      r20, IO_PIN
    sbrc    r20, RX    ; confirm start bit remains low
    rjmp    wait_start

;   Wait a 1 bit period for a total of 1.5 bit periods
    ldi     r19, period
    rcall   timer_delay

;   Read 8 data bits, LSB first, into r17
    ldi     r20, 8              ; bit counter

read_bit:
    in      r18, IO_PIN           ; scratch read - do not clobber r17
    clc                         ; assume bit is 0
    sbrc    r18, RX         ; skip sec when RX is LOW
    sec                         ; RX HIGH -> bit is 1
    ror     r17                 ; shift carry into MSB (LSB-first)

    ldi     r19, period
    rcall   timer_delay

    dec     r20
    brne    read_bit

;   Stop bit check - line must be HIGH (mark); loop fall-through is ~mid stop bit.
;   Status returned in T flag:  T=0 frame OK,  T=1 framing error.
    clt                         ; assume frame OK
    in      r18, IO_PIN
    sbrs    r18, RX         ; skip 'set' when stop bit is HIGH (valid)
    set                         ; stop bit LOW -> framing error
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
