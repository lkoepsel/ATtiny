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

.equ    SYS_CLOCK, PB0          ; OC0A, fires every interrupt, use to measure SYS_CLOCK
.equ    SOFT_TX_PIN, PB1        ; transmit pin, output
.equ    SOFT_RX_PIN, PB2        ; receive pin, input pullup
.equ    TRIM, 0x65              ; OSCCAL trim value, typically x6n
.equ    baud_ticks, 35         ; ticks for one bit period (9600 baud @ 1.2MHz)
.equ    start_ticks, 60        ; ticks for ~1.5 bit periods - tune on scope
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
    eor     r24, r24        ; clear counter low byte

    rjmp    main_setup

; --------------------------------------------------------------------
; main – application logic starts here
; --------------------------------------------------------------------
main_setup:

    rcall   init_soft_serial

main_loop:
    rcall   soft_char_read
    rcall   soft_char_write
    rjmp    main_loop

; ====================================================================
;  Subroutines SECTION
; ====================================================================
; blocking delay, enter with r24 as 8-bit counter value
timer_delay:
; ~1ms delay at 1.2MHz
delay_loop:
    dec    R24
    brne   delay_loop
    ret


init_soft_serial:
;   Set TX pin as output, set RX pin as input pullup
;   DDRB |= _BV(SOFT_TX_PIN);
;   DDRB &= ~_BV(SOFT_RX_PIN);
;   PORTB |= _BV(SOFT_RX_PIN);
;   OSCCAL = 0x73;  use ../osscal routine to determine optimal value
    sbi     DDRB, SOFT_TX_PIN
    cbi     DDRB, SOFT_RX_PIN
    sbi     PORTB, SOFT_RX_PIN
    ; ldi     r16, TRIM           ; osc trim value
    ; out     OSCCAL, r16         ; nudge oscillator toward true 1.2MHz (maybe)
    sbi     PORTB, SOFT_TX_PIN
    ret

soft_char_write:
    ; char to write in r17
    ; ldi     r20, 0x41

    ; Start bit
    cbi     PORTB, SOFT_TX_PIN
    ldi     R24,baud_ticks
    rcall   timer_delay


    ;  Data bits
    ldi     r16, 8

write_bit:
    ror     r17
    brcs    write_one
    cbi     PORTB, SOFT_TX_PIN
    rjmp     next_write

write_one:
    sbi     PORTB, SOFT_TX_PIN

next_write:
    ldi     R24,baud_ticks
    rcall   timer_delay

    dec     r16
    brne    write_bit

    ;  Stop bit
    sbi     PORTB, SOFT_TX_PIN
    ldi     R24,baud_ticks
    rcall   timer_delay
    ret

; soft_char_read - receive one char into r17 (8N1, LSB first)
soft_char_read:
;   Wait for start bit: idle is HIGH, start bit is LOW
;   while (PINB & (1 << SOFT_RX_PIN)) {} ;
wait_start:
    in      r16, PINB
    sbrc    r16, SOFT_RX_PIN    ; skip rjmp when RX is LOW = start bit
    rjmp    wait_start

;   Wait ~1.5 bit periods so bit0 is sampled mid-bit
    ldi     r24, start_ticks
    rcall   timer_delay

;   Read 8 data bits, LSB first, into r17
    ldi     r16, 8              ; bit counter

read_bit:
    in      r18, PINB           ; scratch read - do not clobber r17
    clc                         ; assume bit is 0
    sbrc    r18, SOFT_RX_PIN    ; skip sec when RX is LOW
    sec                         ; RX HIGH -> bit is 1
    ror     r17                 ; shift carry into MSB (LSB-first)

    ldi     r24, baud_ticks
    rcall   timer_delay

    dec     r16
    brne    read_bit
    ret

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
