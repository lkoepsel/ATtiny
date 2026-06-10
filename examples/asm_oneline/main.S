; dispatch.S - ICALL jump table demo for ATtiny13A
; avr-gcc -mmcu=attiny13a -nostdlib -o dispatch.elf dispatch.S

#include <avr/io.h>
#include "registers.S"

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
    reti                        ; 0x006  TIM0_COMPA - Timer0 Compare Match A
    reti                        ; 0x007  TIM0_COMPB - Timer0 Compare Match B
    reti                        ; 0x008  WDT        - Watchdog Time-out
    reti                        ; 0x009  ADC        - ADC Conversion Complete

; ====================================================================
;  TEXT SECTION  (executable code lives here)
; ====================================================================
.section .text

; ---------- Registers and Values ----------------
; r4                            ; saved index
; r7:r6                         ; temp registers for lpm command
; r16                           ; temp register
; r19:r18                       ; ticks register
; ---------- Reserved registers ----------
; r2        ISR_temp - ISR scratch (STATUS save) — do NOT use elsewhere
; r9:r8     ticks_hi/lo - global 16-bit ISR ticks counter — do NOT use elsewhere


; ---- Vector table ----
        .section .text
        .global  main
reset_handler:

        ; ATtiny13A has only SPL (no SPH) — RAMEND = 0x9F fits in 8 bits
        ldi     temp_16, lo8(RAMEND)
        out     STACK_LOW, temp_16

        ; r1 = 0 by convention (zero register); clear status flags
        eor     r1, r1
        out     STATUS, r1

; --------------------------------------------------------------------
; main – 
; --------------------------------------------------------------------

main:
        ldi     temp_16, 0xFF
        out     _SFR_IO_ADDR(DDRB), temp_16   ; PORTB all outputs

reload:
        ; ---- Choose an index (0..7) and dispatch ----
        ldi     temp_16, 0x0C                    ; initial index into the table
dispatch:
        mov     r4, temp_16                      ; save it for next iteration
        ; Z = jump_table + (index * 1 word)
        ldi     ZL, lo8(jump_table)
        ldi     ZH, hi8(jump_table)
        ; add index*2 BYTES to the byte pointer used by LPM...
        lsl     temp_16                       ; index * 2 (each entry is 1 word = 2 bytes)
        add     ZL, temp_16
        adc     ZH, r1                    ; r1 assumed 0; clr it if unsure

        ; Load the task's WORD address stored in the table
        lpm     r6, Z+
        lpm     r7, Z
        mov     ZL, r6                   ; low byte
        mov     ZH, r7                    ; high byte
        ; Z now holds the word-address of the target task
        icall                             ; push PC, jump to (Z)

        mov     temp_16, r4
        dec     temp_16
        brmi    reload
        rjmp    dispatch


; ---- The tasks ----
task0:
        ldi     temp_16, 0x00
        out     _SFR_IO_ADDR(PORTB), temp_16
        delay_16 0x1000
        ret

task1:
        ldi     temp_16, 0x01
        out     _SFR_IO_ADDR(PORTB), temp_16
        delay_16 0x800
        ret

task2:
        ldi     temp_16, 0x02
        out     _SFR_IO_ADDR(PORTB), temp_16
        delay_16 0x600
        ret

task3:
        ldi     temp_16, 0x03
        out     _SFR_IO_ADDR(PORTB), temp_16
        delay_16 0x800
        ret

task4:
        ldi     temp_16, 0x04
        out     _SFR_IO_ADDR(PORTB), temp_16
        delay_16 0x400
        ret

task5:
        ldi     temp_16, 0x05
        out     _SFR_IO_ADDR(PORTB), temp_16
        delay_16 0x200
        ret

task6:
        ldi     temp_16, 0x06
        out     _SFR_IO_ADDR(PORTB), temp_16
        delay_16 0x500
        ret

task7:
        ldi     temp_16, 0x07
        out     _SFR_IO_ADDR(PORTB), temp_16
        delay_16 0x1000
        ret

; ---- The table: each entry is the WORD address of a task ----
        .balign 2
jump_table:
        .word   pm(task0)
        .word   pm(task5)
        .word   pm(task1)
        .word   pm(task3)
        .word   pm(task7)
        .word   pm(task4)
        .word   pm(task0)
        .word   pm(task6)
        .word   pm(task2)
        .word   pm(task1)
        .word   pm(task3)
        .word   pm(task4)
        .word   pm(task6)

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

