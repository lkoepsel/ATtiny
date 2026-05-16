; =============================================================
; main.asm  –  Blink LED on PB0 at 500Hz (1ms on, 1ms off)
; Target : ATtiny13A at 1.2MHz (factory default clock)
; Circuit: LED + 220Ω resistor from PB0 (pin 5) to GND
; ---------- Timer setup -------------------
; Internal RC-Oscillator = 9,600,000 Hz
; Clock prescaler CLKPR = 8
; Internal contr. clock = 1,200,000 Hz
; TC0 precaler = 1,024
; TC0 tick = 1,171.875 cs/s
; TC0 cycle = 256
; TC0 single cycle = 4.578 cs/s
; TC0 toggle frequency = 2.289 cs/s
; =============================================================

.include "tn13Adef.inc"

.section .vectors, "ax", @progbits
    rjmp    reset_handler       ; 0x000  RESET
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

reset_handler:                  ; also serves as main_setup
    ldi     r16, lo8(RAMEND)    ; init stack
    out     SPL, r16            ; ATtiny13A has no SPH
    eor     r1, r1
    out     SREG, r1            ; clear status register
    sbi     DDRB, PB0           ; PB0 as output

    ; Select Compare Match
    ldi     r18,0xFF            ; Match at 255
    out     OCR0A,R18           ; to Compare Match A

    ; toggle PB0 at Compare Match
    ldi     r18,1<<COM0A0       ; Toggle Mode
    out     TCCR0A,R18          ; to control port A

    ; Start timer
    ldi     r18,(1<<CS02)|(1<<CS00) ; prescaler 1024
    out     TCCR0B,R18          ; to control port B

Loop:
    rjmp Loop

.section .data
.section .bss
