; =============================================================
; main.asm  –  Blink LED on PB0 at ~0.5Hz (1s on, 1s off)
; Target : ATtiny13A at 1.2MHz (factory default clock)
; Circuit: LED + 220Ω resistor from PB0 (pin 5) to GND
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

reset_handler:
    ldi     r16, lo8(RAMEND)
    out     SPL, r16            ; ATtiny13A has no SPH
    eor     r1, r1
    out     SREG, r1
    sbi     DDRB, PB0           ; PB0 as output

main_loop:
    sbi     PINB, PB0           ; toggle PB0 — writing 1 to PINB flips PORTB output
    rcall   delay_1s
    rjmp    main_loop

; ~998ms delay at 1.2MHz
; cycles = 8 × (195 × (3×255 + 3) + 4) = 1,198,112 ≈ 998ms
delay_1s:
    ldi     r18, 8
outer:
    ldi     r17, 195
middle:
    ldi     r16, 255
inner:
    dec     r16
    brne    inner
    dec     r17
    brne    middle
    dec     r18
    brne    outer
    ret

.section .data
.section .bss
