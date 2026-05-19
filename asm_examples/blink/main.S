; =============================================================
; main.asm  –  Blink LED on PB0 at 500Hz (1ms on, 1ms off)
; Target : ATtiny13A at 1.2MHz (factory default clock)
; Circuit: LED + 220Ω resistor from PB0 (pin 5) to GND
; =============================================================

#include <avr/io.h>
#include "registers.h"

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

reset_handler:                  ; also serves as main_setup
    ldi     r16, lo8(RAMEND)    ; init stack
    out     STACK_LOW, r16      ; ATtiny13A has no SPH
    eor     r1, r1
    out     STATUS, r1          ; clear status register
    sbi     LED_DDRB, PB0       ; PB0 as output

main_loop:
    sbi     LED_PINB, PB0       ; toggle PB0 — writing 1 to PINB flips PORTB output
    rcall   delay_1ms
    rjmp    main_loop

; ~1ms delay at 1.2MHz
; cycles = 2 × (3×189 + 3) + 4) ~= 1ms (1.004ms measured)
delay_1ms:
    ldi     r17, 2
outer:
    ldi     r16, 189
inner:
    dec     r16
    brne    inner
    dec     r17
    brne    outer
    ret

.section .data
.section .bss
