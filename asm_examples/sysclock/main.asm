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
    rjmp    TIM0_COMPA_handler  ; 0x006  TIM0_COMPA - Timer0 Compare Match A
    reti                        ; 0x007  TIM0_COMPB - Timer0 Compare Match B
    reti                        ; 0x008  WDT        - Watchdog Time-out
    reti                        ; 0x009  ADC        - ADC Conversion Complete

; ====================================================================
;  TEXT SECTION  (executable code lives here)
; ====================================================================
.section .text

; ---------- Registers and Values ----------------
; r16                           ; temp register
; R25:R24 reserved as global 16-bit ISR counter
; Do NOT use R24 or R25 anywhere else in your code

.equ    SYS_CLOCK, PB0          ; OC0A, fires every interrupt, use to measure SYS_CLOCK
.equ    SOFT_TX_PIN, PB1        ; transmit pin, output
.equ    SOFT_RX_PIN, PB2        ; receive pin, input pullup
.equ    TRIM, 0x65              ; OSCCAL trim value, typically x6n
.equ    baud_ticks, 156         ; number of ticks for 1200 baud
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
    eor     r25, r25        ; clear counter high byte

    rjmp    main_setup

; --------------------------------------------------------------------
; main – application logic starts here
; --------------------------------------------------------------------
main_setup:

    ; rcall     init_soft_serial
    rcall     init_sysclock_1k

main_loop:
    ; Put your program logic here
    rjmp    main_loop

; ====================================================================
;  Subroutines SECTION
; ====================================================================
TIM0_COMPA_handler:
    push    r16             ; save temp register
    in      r16, SREG       ; save status flags
    push    r16

    adiw    R24, 1          ; increment global counter — NOT saved/restored

    pop     r16
    out     SREG, r16
    pop     r16
    reti


init_sysclock_1k:
;      Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
;      The values below will result in a 1kHz counter (1000 ticks = 1 second)

;   WGM01 CTC mode, OCR0A is TOP, toggle PB0 on Compare Match
    ldi     r16, (1<<COM0A0) | (1<<WGM01)
    out     TCCR0A,R16

;   / 8 prescalar
    ldi     r16, (1<<CS01)      ;
    out     TCCR0B,r16          ;

;   w/ sei and OCIE0A set, Timer/Counter0 Compare Match A interrupt is enabled
    ldi     r16, (1<<OCIE0A)      ;
    out     TIMSK0,r16          ;

    ; OCR0A: adjust for a 1kHz signal (998.8kHz measured)
    ldi     r18,0x47       ;
    out     OCR0A,r18           ;
    sei
    sbi     DDRB, PB0           ; PB0 as output, for checking SYS_CLOCK
    ret

init_soft_serial:
;   Set TX pin as output, set RX pin as input pullup
;   DDRB |= _BV(SOFT_TX_PIN);
;   DDRB &= ~_BV(SOFT_RX_PIN);
;   PORTB |= _BV(SOFT_RX_PIN);
;   OSCCAL = 0x73;  use ../osscal routine to determine optimal value
;   TCCR0B = (1 << CS01);   Prescaler /8
    sbi     DDRB, SOFT_TX_PIN
    cbi     DDRB, SOFT_RX_PIN
    sbi     PORTB, SOFT_RX_PIN
    ldi     r16, TRIM           ; osc trim value
    out     OSCCAL, r16         ; nudge oscillator toward true 1.2MHz (maybe)

    ret

soft_char_write:
        ; Start bit
        ; PORTB &= ~(1 << SOFT_TX_PIN);
        ; TIMER_DELAY(baud_ticks);

        ;  Data bits
        ; for (uint8_t i = 0; i < 8; i++)
        ; {
        ;     if (data & (1 << i))
        ;     {
        ;         PORTB |= (1 << SOFT_TX_PIN);
        ;     }
        ;     else
        ;     {
        ;         PORTB &= ~(1 << SOFT_TX_PIN);
        ;     }
        ;     TIMER_DELAY(baud_ticks);
        ; }

        ;  Stop bit
        ; PORTB |= (1 << SOFT_TX_PIN);
        ; TIMER_DELAY(baud_ticks);


; ====================================================================
;  DATA SECTION  (initialized variables in SRAM)
;  Declare with:  my_var: .byte 0
; ====================================================================
.section .data
ticks_ctr:  .word   1
; ====================================================================
;  BSS SECTION  (zero-initialized / uninitialized variables in SRAM)
;  Declare with:  my_buf: .skip 16
; ====================================================================
.section .bss
