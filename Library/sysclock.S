; ====================================================================
;  sysclock
;       TIM0_COMPA_handler - minimal ISR to increment ticks counter
;       init_sysclock_1k - setup timer to provide 1ms ticks
; Target : ATtiny13A (1.2 MHz default internal RC oscillator)
; Toolchain: avr-as / avr-ld  (GNU Binutils for AVR)
; =============================================================

; Dependencies — included here so this module assembles standalone

#include <avr/io.h>
#include "registers.S"

.section .vectors, "ax", @progbits
    ; rjmp    reset_handler       ; 0x000  RESET
    ; reti                        ; 0x001  INT0       - External Interrupt 0
    ; reti                        ; 0x002  PCINT0     - Pin Change Interrupt 0
    ; reti                        ; 0x003  TIM0_OVF   - Timer/Counter0 Overflow
    ; reti                        ; 0x004  EE_RDY     - EEPROM Ready
    ; reti                        ; 0x005  ANA_COMP   - Analog Comparator
    rjmp    TIM0_COMPA_handler  ; 0x006  TIM0_COMPA - Timer0 Compare Match A
    ; reti                        ; 0x007  TIM0_COMPB - Timer0 Compare Match B
    ; reti                        ; 0x008  WDT        - Watchdog Time-out
    ; reti                        ; 0x009  ADC        - ADC Conversion Complete

; ====================================================================
;  TEXT SECTION  (executable code lives here)
; ====================================================================
.section .text

; ---------- Registers and Values ----------------
; r16                           ; temp register - temp_r18
; r20                           ; temp register - bit_ctr
; r24                           ; char register - char_reg

; ====================================================================
;  Subroutines SECTION
; ====================================================================

TIM0_COMPA_handler:
    in      ISR_temp, STATUS        ; save status flags
    inc     ticks_lo                ; r8++
    brne    done                    ; no carry → done
    inc     ticks_hi                ; carry → bump high byte
done:
    out     STATUS, ISR_temp
    reti

.global init_sysclock_1k
init_sysclock_1k:
;   Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
;   The values below will result in a 1kHz counter (1000 ticks = 1 second)
;   TCRR0A 0b0100 0010 0x42     Toggle OC0A on Compare Match, CTC
;   TCRR0B 0b0000 0010 0x02     /8 Prescalar
;   TIMS0K 0b0000 0100 0x04     Output Compare Match A Interrupt Enable
;   OCR0A  0b1001 0101 0x95     Adjusted for a 1kHz signal

; clear counter register
    eor     ticks_lo, ticks_lo        ; clear counter low byte
    eor     ticks_hi, ticks_hi        ; clear counter high byte

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
    ldi     r16,0x95       ;
    out     OCRA,r16           ;
    sbi     IO_DDR, LED           ; PB0 as output, for checking SYS_CLOCK
    sei
    ret

.global ticks
ticks:
    movw r24, ticks_lo
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

