; Register and other definitions required for assembly files
#ifndef REGISTERS_S
#define REGISTERS_S
#define LED         PB0                     ; LED
#define TX          PB3                     ; transmit pin, output
#define RX          PB4                     ; receive pin, input pullup
#define DEBUG       PB4                     ; receive pin, input pullup
#define IO_PORT    _SFR_IO_ADDR(PORTB)
#define IO_DDR      _SFR_IO_ADDR(DDRB)
#define IO_PIN     _SFR_IO_ADDR(PINB)
#define STACK_LOW   _SFR_IO_ADDR(SPL)
#define STATUS      _SFR_IO_ADDR(SREG)
#define OCRA        _SFR_IO_ADDR(OCR0A)
#define TCCRA       _SFR_IO_ADDR(TCCR0A)
#define TCCRB       _SFR_IO_ADDR(TCCR0B)
#define TIMSK       _SFR_IO_ADDR(TIMSK0)
#define RCCAL       _SFR_IO_ADDR(OSCCAL)
#define ISR_temp    r2
#define ticks_lo    r8
#define ticks_hi    r9
#define temp_r18    r18
#define bit_ctr     r20
#define char_reg    r24
#define delay_lo    r26
#define delay_hi    r27
#define flash_lo    r30
#define flash_hi    r31

.macro  delay_8  ticks
    ldi     r19, \ticks
9:  dec     r19
    brne    9b
.endm

.macro  delay_16  ticks
  ldi     r18, lo8(\ticks)
  ldi     r19, hi8(\ticks)
9:  subi    r18, 1
  sbci    r19, 0
  brne    9b
.endm

#endif  /* REGISTERS_S */
