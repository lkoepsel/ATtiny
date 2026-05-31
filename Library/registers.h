#define LED         PB0                     ; LED
#define TX          PB1                     ; transmit pin, output
#define RX          PB2                     ; receive pin, input pullup
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


.macro  delay_ticks  ticks
    ldi     r19, \ticks
9:  dec     r19
    brne    9b
.endm
