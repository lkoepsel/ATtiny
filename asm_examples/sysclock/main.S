; =============================================================
; sysclock  –  provides non-blocking 1kHz timer
; Target : ATtiny13A (1.2 MHz default internal RC oscillator)
; Toolchain: avr-as / avr-ld  (GNU Binutils for AVR)
; =============================================================

#include <avr/io.h>
#include "registers.h"
.include "soft_serial.S"

; ====================================================================
;  INTERRUPT VECTOR TABLE
;  ATtiny13A has 10 vectors, each one instruction word (2 bytes) wide.
;  Must live at flash address 0x0000.
; ====================================================================
.section .vectors, "ax", @progbits
    rjmp    reset_handler         ; 0x000  RESET
    ; reti                        ; 0x001  INT0       - External Interrupt 0
    ; reti                        ; 0x002  PCINT0     - Pin Change Interrupt 0
    ; reti                        ; 0x003  TIM0_OVF   - Timer/Counter0 Overflow
    ; reti                        ; 0x004  EE_RDY     - EEPROM Ready
    ; reti                        ; 0x005  ANA_COMP   - Analog Comparator
    rjmp    TIM0_COMPA_handler    ; 0x006  TIM0_COMPA - Timer0 Compare Match A
    ; reti                        ; 0x007  TIM0_COMPB - Timer0 Compare Match B
    ; reti                        ; 0x008  WDT        - Watchdog Time-out
    ; reti                        ; 0x009  ADC        - ADC Conversion Complete

; ====================================================================
;  TEXT SECTION  (executable code lives here)
; ====================================================================
.section .text

; ---------- Registers and Values ----------------
; r15                           ; temp register, saved in ISR
; r16                           ; temp register
; ---------- Reserved registers ----------
; r2        ISR scratch (STATUS save) — do NOT use elsewhere
; R25:R24   global 16-bit ISR counter — do NOT use elsewhere

.equ    COUNTER,  30000         ; 30,000@1.2MHZ => 100ms delay
.equ    TRIM, 0x65              ; OSCCAL trim value, typically x6n
.equ    HEX_BEG, 0xBB           ; beginning of hex print
.equ    HEX_END, 0xEE           ; end of hex print
; --------------------------------------------------------------------
; reset_handler – entry point after RESET
; --------------------------------------------------------------------
reset_handler:

    ; ATtiny13A has only SPL (no SPH) — RAMEND = 0x9F fits in 8 bits
    ldi     r16, lo8(RAMEND)
    out     STACK_LOW, r16

    ; r1 = 0 by convention (zero register); clear status flags
    eor     r1, r1
    out     STATUS, r1

; --------------------------------------------------------------------
; main – application logic starts here
; --------------------------------------------------------------------
main_setup:

    rcall   init_sysclock_1k
    rcall   init_serial

main_loop:
    ldi     r17, HEX_BEG
    rcall   char_write

    ldi     r27,hi8(COUNTER)
    ldi     r26,lo8(COUNTER)

    ; get clock start
    movw    r4, r24

delay_1ms:
    sbiw    r26,1
    brne    delay_1ms

    ; get clock end
    movw    r6, r24

    ; delta = end - start
    sub     r6, r4
    sbc     r7, r5

    ; print delta
    mov     r17, r7
    rcall   char_write
    mov     r17, r6
    rcall   char_write

    ldi     r17,  HEX_END
    rcall   char_write

    rjmp    main_loop

; ====================================================================
;  Subroutines SECTION
; ====================================================================
TIM0_COMPA_handler:
    in      r2, STATUS      ; save status flags
    adiw    r24, 1           ; increment global counter — NOT saved/restored
    out     STATUS, r2      ; restore status flags
    reti


init_sysclock_1k:
;      Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
;      The values below will result in a 1kHz counter (1000 ticks = 1 second)

    ; clear counter register
    eor     r24, r24        ; clear counter low byte
    eor     r25, r25        ; clear counter high byte

    ;   WGM01 CTC mode, OCR0A is TOP, toggle LED on Compare Match
    ldi     r16, (1<<COM0A0) | (1<<WGM01)
    out     TCCRA,R16

;   / 8 prescalar
    ldi     r16, (1<<CS01)      ;
    out     TCCRB,r16          ;

;   w/ sei and OCIE0A set, Timer/Counter0 Compare Match A interrupt is enabled
    ldi     r16, (1<<OCIE0A)      ;
    out     TIMSK,r16          ;

    ; OCR0A: adjust for a 1kHz signal (998.8kHz measured)
    ldi     r18,0x95       ;
    out     OCRA,r18           ;
    sei
    sbi     IO_DDR, LED           ; PB0 as output, for checking SYS_CLOCK
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
