; =============================================================
; main.asm  –  Minimal AVR Assembly Skeleton
; Target : ATtiny13A (1.2 MHz default internal RC oscillator)
; Toolchain: avr-as / avr-ld  (GNU Binutils for AVR)
; =============================================================

; ----- Device constants -----
.equ    RAMEND,   0x9F       ; Top of SRAM (64 bytes: 0x60–0x9F)

; ----- I/O Register addresses (I/O space, used with IN/OUT/SBI/CBI) -----
.equ    PINB,     0x16       ; Port B Input Pins
.equ    DDRB,     0x17       ; Data Direction Register B
.equ    PORTB,    0x18       ; Port B Data Register
.equ    SREG,     0x3F       ; Status Register

; ====================================================================
;  INTERRUPT VECTOR TABLE
;  ATtiny13A has 10 vectors, each one instruction word (2 bytes) wide.
;  Must live at flash address 0x0000.
; ====================================================================
.section .vectors, "ax", @progbits
    rjmp    reset_handler       ; 0x000  RESET
    rjmp    default_isr         ; 0x001  INT0       - External Interrupt 0
    rjmp    default_isr         ; 0x002  PCINT0     - Pin Change Interrupt 0
    rjmp    default_isr         ; 0x003  TIM0_OVF   - Timer/Counter0 Overflow
    rjmp    default_isr         ; 0x004  EE_RDY     - EEPROM Ready
    rjmp    default_isr         ; 0x005  ANA_COMP   - Analog Comparator
    rjmp    default_isr         ; 0x006  TIM0_COMPA - Timer0 Compare Match A
    rjmp    default_isr         ; 0x007  TIM0_COMPB - Timer0 Compare Match B
    rjmp    default_isr         ; 0x008  WDT        - Watchdog Time-out
    rjmp    default_isr         ; 0x009  ADC        - ADC Conversion Complete

; ====================================================================
;  TEXT SECTION  (executable code lives here)
; ====================================================================
.section .text

; --------------------------------------------------------------------
; default_isr – catch-all for unhandled interrupts
; --------------------------------------------------------------------
default_isr:
    reti

; --------------------------------------------------------------------
; reset_handler – entry point after RESET
; --------------------------------------------------------------------
reset_handler:

    ; 1. Initialize Stack Pointer
    ;    ATtiny13A has only SPL (no SPH) — RAMEND = 0x9F fits in 8 bits
    ldi     r16, lo8(RAMEND)
    out     0x3D, r16           ; SPL = RAMEND

    ; 2. Clear SREG (status/flag register)
    eor     r1, r1              ; r1 = 0  (zero register by convention)
    out     SREG, r1

    rjmp    main

; --------------------------------------------------------------------
; main – application logic starts here
; --------------------------------------------------------------------
main:
    ; Configure PB0 (pin 5) as OUTPUT
    ; Avoid PB5 (pin 1) — that is the RESET pin
    ldi     r16, (1 << 0)
    out     DDRB, r16

main_loop:
    ; Put your program logic here
    rjmp    main_loop

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

; ====================================================================
;  END OF FILE
; ====================================================================
