; =============================================================
; main.asm  –  Output 100ms signal on PB0
; Target : ATtiny13A at 1.2MHz (factory default clock)
; Circuit: LED + 220Ω resistor from PB0 (pin 5) to GND
; ---------- Timer setup -------------------
; Internal RC-Oscillator = 9,600,000 Hz
; Clock prescaler CLKPR = 8
; Internal contr. clock = 1,200,000 Hz
; TC0 precaler = 1,024
; TC0 tick = 1,171.875 /s
; =============================================================

.include "tn13Adef.inc"

.section .vectors, "ax", @progbits
    rjmp    main_setup       ; 0x000  RESET
    reti                        ; 0x001  INT0
    reti                        ; 0x002  PCINT0
    reti                        ; 0x003  TIM0_OVF
    reti                        ; 0x004  EE_RDY
    reti                        ; 0x005  ANA_COMP
    reti                        ; 0x006  TIM0_COMPA
    reti                        ; 0x007  TIM0_COMPB
    reti                        ; 0x008  WDT
    reti                        ; 0x009  ADC

.section .text

; ---------- Registers ----------------
; R18                           ; Multi purpose register

.equ    DC, 50                  ; duty cycle (%)
.equ    DC_FACTOR, (DC * 255) / 100    ; calculation for duty cycle

main_setup:                  ; also serves as main_setup
    ldi     r16, lo8(RAMEND)    ; init stack
    out     SPL, r16            ; ATtiny13A has no SPH
    eor     r1, r1
    out     SREG, r1            ; clear status register
    sbi     DDRB, PB0           ; PB0 as output

    ; TCCR0A: Toggle PB0 at Compare Match to OCR0A (CTC mode)
    ldi     r18,(1<<COM0A0) | (1<<WGM00)
    out     TCCR0A,R18

    ; TCCR0B: 1/1024 prescaler
    ldi     r18, (1<<WGM02) | (1<<CS02)|(1<<CS00) ; prescaler 1024
    out     TCCR0B,R18          ; to control port B

    ; OCR0A: Use DC_FACTOR for Compare Match
    ldi     r18,DC_FACTOR         ; Match at DC_FACTOR
    out     OCR0A,R18           ;

main_loop:
    rjmp main_loop

.section .data
.section .bss
