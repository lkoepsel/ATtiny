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

TIM0_COMPA_handler:
    in      ISR_temp, STATUS        ; save status flags
    inc     ticks_lo                ; r8++
    brne    done                    ; no carry → done
    inc     ticks_hi                ; carry → bump high byte
done:
    out     STATUS, ISR_temp
    reti


init_sysclock_1k:
;   Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
;   The values below will result in a 1kHz counter (1000 ticks = 1 second)
;   TCRR0A 0b01000010 0x42
;   TCRR0B 0b00000010 0x02
;   TIMS0K 0b00000100 0x04
;   OCR0A  0b10010101 0x95

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
    sei
    sbi     IO_DDR, LED           ; PB0 as output, for checking SYS_CLOCK
    ret

