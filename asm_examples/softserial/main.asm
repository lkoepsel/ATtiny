; =============================================================
; softserial  –  serial program and library
; Target : ATtiny13A (1.2 MHz default internal RC oscillator)
; Toolchain: avr-as / avr-ld  (GNU Binutils for AVR)
; =============================================================

.include "tn13Adef.inc"

; ====================================================================
;  INTERRUPT VECTOR TABLE
;  ATtiny13A has 10 vectors, each one instruction word (2 bytes) wide.
;  Must live at flash address 0x0000.
; ====================================================================
.section .vectors, "ax", @progbits
    rjmp    reset_handler       ; 0x000  RESET
    reti                        ; 0x001  INT0       - External Interrupt 0
    reti                        ; 0x002  PCINT0     - Pin Change Interrupt 0
    reti                        ; 0x003  TIM0_OVF   - Timer/Counter0 Overflow
    reti                        ; 0x004  EE_RDY     - EEPROM Ready
    reti                        ; 0x005  ANA_COMP   - Analog Comparator
    reti                        ; 0x006  TIM0_COMPA - Timer0 Compare Match A
    reti                        ; 0x007  TIM0_COMPB - Timer0 Compare Match B
    reti                        ; 0x008  WDT        - Watchdog Time-out
    reti                        ; 0x009  ADC        - ADC Conversion Complete

; ====================================================================
;  TEXT SECTION  (executable code lives here)
; ====================================================================
.section .text

; ---------- Registers and Values ----------------
; r16                           ; temp register
; r17                           ; char register
; r18                           ; temp register
; r24                           ; timer delay register

.equ    TX_PIN, PB1             ; transmit pin, output
.equ    RX_PIN, PB2             ; receive pin, input pullup
.equ    period, 35              ; ticks for 1 bit period (9600 baud @ 1.2MHz)
.equ    half_period, 18         ; ticks for  a .5 bit period
; --------------------------------------------------------------------
; reset_handler – entry point after RESET
; --------------------------------------------------------------------
reset_handler:

    ; ATtiny13A has only SPL (no SPH) — RAMEND = 0x9F fits in 8 bits
    ldi     r16, lo8(RAMEND)
    out     SPL, r16

    ; r1 = 0 by convention (zero register); clear status flags
    eor     r1, r1
    out     SREG, r1

    rjmp    main_setup

; --------------------------------------------------------------------
; main – application logic starts here
; --------------------------------------------------------------------
main_setup:

    rcall   init_serial

; simple echo loop to test read/write char
main_loop:
    rcall   char_read
    rcall   char_write
    rjmp    main_loop

; ====================================================================
;  Subroutines SECTION
; ====================================================================
; blocking delay, enter with r24 as 8-bit counter value
timer_delay:
    dec    r24
    brne   timer_delay
    ret
; --------------------------------------------------------------------

;initialize serial port, must be called before using char_read and char_write
init_serial:
;   Set TX pin as output, set RX pin as input pullup
    sbi     DDRB, TX_PIN
    cbi     DDRB, RX_PIN
    sbi     PORTB, RX_PIN

    ; set TX_PIN high to start, start_bit is low
    sbi     PORTB, TX_PIN
    ret
; --------------------------------------------------------------------

; write a char in r17 to serial port, r17 is preserved
char_write:

    ; Start bit
    cbi     PORTB, TX_PIN
    ldi     r24,period
    rcall   timer_delay


    ;  8 data bits and preserve char
    ldi     r16, 8
    mov     r18, r17

write_bit:
    ror     r18
    brcs    write_one
    cbi     PORTB, TX_PIN
    rjmp     next_write

write_one:
    sbi     PORTB, TX_PIN

next_write:
    ldi     r24,period
    rcall   timer_delay

    dec     r16
    brne    write_bit

    ;  Stop bit
    sbi     PORTB, TX_PIN
    ldi     r24,period
    rcall   timer_delay
    ret
; --------------------------------------------------------------------


; char_read - receive one char into r17 (8N1, LSB first)
char_read:
;   Wait for start bit: idle is HIGH, start bit is LOW
;   while (PINB & (1 << RX_PIN)) {} ;
wait_start:
    in      r16, PINB
    sbrc    r16, RX_PIN    ; skip rjmp when RX is LOW = start bit
    rjmp    wait_start

;   Wait a .5 bit period so bit0 is sampled mid-bit
    ldi     r24, half_period
    rcall   timer_delay

    in      r16, PINB
    sbrc    r16, RX_PIN    ; confirm start bit remains low
    rjmp    wait_start

;   Wait a 1 bit period for a total of 1.5 bit periods
    ldi     r24, period
    rcall   timer_delay

;   Read 8 data bits, LSB first, into r17
    ldi     r16, 8              ; bit counter

read_bit:
    in      r18, PINB           ; scratch read - do not clobber r17
    clc                         ; assume bit is 0
    sbrc    r18, RX_PIN         ; skip sec when RX is LOW
    sec                         ; RX HIGH -> bit is 1
    ror     r17                 ; shift carry into MSB (LSB-first)

    ldi     r24, period
    rcall   timer_delay

    dec     r16
    brne    read_bit

    ; consume stop bit
    ldi     r24, period
    rcall   timer_delay

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
