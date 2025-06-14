; does not compile, more work to do
.org 0x0000
rjmp start
start:
    ldi r16, 0 ; reset system status
    out SREG, r16 ; init stack pointer
    ldi r16, low(RAMEND)
    out SPL, r16
    ldi r16, high(RAMEND)
    out SPH, r16
    sbi DDRB, DDB5 ;pinMode(13, OUTPUT);
_loop:
    sbi PORTB, PORTB5 ;turn LED on
    rcall _delay
    cbi PORTB, PORTB5 ;turn LED off
    rcall _delay
    rjmp _loop
_delay:
    ldi r24, 0x00 ;one second delay iteration
    ldi r23, 0xd4
    ldi r22, 0x30
_d1: 
    subi r24, 1
    sbci r23, 0
    sbci r22, 0
    brcc _d1
    ret
