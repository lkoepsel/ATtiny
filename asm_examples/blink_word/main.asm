; =============================================================
; main.asm  –  Blink LED on PB0 at 500Hz (1ms on, 1ms off)
; Use 16-bit counter
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

.equ    counter,  50000     ; 1 to 65535 (does not generate code)

reset_handler:                  ; also serves as main_setup
    eor     r1, r1
    out     SREG, r1            ; clear status register
    sbi     DDRB, PB0           ; PB0 as output

main_loop:
    ldi     R25,hi8(counter)    ; 1 clock cycle, executed once
    ldi     R24,lo8(counter)    ; 1 clock cycle, executed once
    sbi     PINB, PB0           ; toggle PB0 — writing 1 to PINB flips PORTB output

    ; ~1ms delay at 1.2MHz
    ; TODO: cycles = 2 × (3×189 + 3) + 4) ~= 1ms (1.004ms measured)
    delay_1ms:
        sbiw R24,1              ; count down 16 bit, 2 clock cycles, executed 50000 times
        brne delay_1ms
    rjmp    main_loop

.section .data
.section .bss
