; =============================================================
; serial  –  serial program and library
; Target : ATtiny13A (1.2 MHz default internal RC oscillator)
; Toolchain: avr-as / avr-ld  (GNU Binutils for AVR)
; =============================================================

; ====================================================================
;  TEXT SECTION  (executable code lives here)
; ====================================================================
.section .text

; ---------- Registers and Values ----------------
; r18                           ; temp register - temp_r18
; r20                           ; temp register - bit_ctr
; r24                           ; char register - char_reg

#define    period       37      ; # of ticks for 1 bit period (9600 baud @ 1.2MHz)
#define    half_period  20      ; # of ticks for  a .5 bit period
#define    TRIM         0x60    ; OSCCAL trim value, use examples/osccal to determine

; ====================================================================
;  Subroutines SECTION
; ====================================================================

;initialize serial port, must be called before using char_read and char_write
.global init_serial
init_serial:

    ldi     temp_r18, TRIM           ; osc trim value
    out     RCCAL, temp_r18          ; apply measured OSCCAL trim (via examples/osccal)

;   Set TX pin as output, set RX pin as input pullup
    sbi     IO_DDR, TX
    cbi     IO_DDR, RX
    sbi     IO_PORT, RX

    ; set TX high to start, start_bit is low
    sbi     IO_PORT, TX
    ret
; --------------------------------------------------------------------

; write a char (passed in r24, per AVR-GCC ABI) to the serial port
.global char_write
char_write:

    ; Start bit
    cbi     IO_PORT, TX
    delay_ticks period

    ;  8 data bits and preserve char
    ldi     bit_ctr, 8
    mov     temp_r18, char_reg

write_bit:
    ror     temp_r18
    brcs    write_one
    cbi     IO_PORT, TX
    rjmp     next_write

write_one:
    sbi     IO_PORT, TX

next_write:
    delay_ticks period

    dec     bit_ctr
    brne    write_bit

    ;  Stop bit
    sbi     IO_PORT, TX
    delay_ticks period
    ret
; --------------------------------------------------------------------


; char_read - receive one char into r24 (8N1, LSB first), per AVR-GCC ABI
.global char_read
char_read:
;   Wait for start bit: idle is HIGH, start bit is LOW
;   while (IO_PIN & (1 << RX)) {} ;
wait_start:
    in      bit_ctr, IO_PIN
    sbrc    bit_ctr, RX    ; skip rjmp when RX is LOW = start bit
    rjmp    wait_start

;   Wait a .5 bit period so bit0 is sampled mid-bit
    delay_ticks half_period

    in      bit_ctr, IO_PIN
    sbrc    bit_ctr, RX    ; confirm start bit remains low
    rjmp    wait_start

;   Wait a 1 bit period for a total of 1.5 bit periods
    delay_ticks period

;   Read 8 data bits, LSB first, into r24
    ldi     bit_ctr, 8              ; bit counter

read_bit:
    in      temp_r18, IO_PIN           ; scratch read - do not clobber r24
    clc                         ; assume bit is 0
    sbrc    temp_r18, RX         ; skip sec when RX is LOW
    sec                         ; RX HIGH -> bit is 1
    ror     char_reg                 ; shift carry into MSB (LSB-first)

    delay_ticks period

    dec     bit_ctr
    brne    read_bit

    ret
; --------------------------------------------------------------------


; void flash_write(uint8_t addr)
;   Set the Z  r31/r30 address to PROGMEM text on entry
;   Calling convention: AVR-GCC ABI (r31/r30 FLASH address, r24 out, r18 scratch).

.global flash_write
flash_write:
        lpm     char_reg, Z+     ; load byte at Z into r24, the char_write register
        tst     char_reg          ; exit if null byte
        breq    print_done
        rcall   char_write  ; write char to console
        rjmp    flash_write

print_done:
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
