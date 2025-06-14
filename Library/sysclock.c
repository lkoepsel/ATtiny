#include "sysclock.h"
#include <util/atomic.h>
#include <avr/interrupt.h>

// ****Defined Interrupt Service Routines****
#if TICKS == 1000
volatile uint32_t ticks_ctr = 0;
#endif

#if TICKS == 100
volatile uint8_t ticks_ctr = 0;
#endif

// Required for ticks timing, see examples/ticks
// Enabled by init_sysclock_1() in sysclock.c
ISR (TIM0_COMPA_vect)      
{
    ticks_ctr++;
}

// ****End of Defined Interrupt Service Routines****

// ****Defined Timer Setup Functions****
#if TICKS == 1000
void init_sysclock_1k (void)          
{
    // Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
    // The values below will result in a 1kHz counter (1000 ticks = 1 second)

    // TCCR0A [ COM0A1 COM0A0 COM0B1 COM0B0 0 0 WGM01 WGM00 ] = 0b00000010
    // TCCR0B [ FOC0A FOC0B 0 0 WGM02 CS02 CS01 CS00 ] = 0b00001010
    // TIMSK0 [ 0 0 0 0  OCIE0B  OCIE0A  TOIE0 0 ] = 0b00000100
    // OCR0A = 0x9a
    // tick = 1/1000 second
    // Test using example/millis _delay_ms(1000); = 1000 ticks

    TCCR0A = ( _BV(WGM01) ) ; 
    TCCR0B |= ( _BV(WGM02) | _BV(CS01) ) ;
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 0x9c;
    sei();
}
#endif

#if TICKS == 100
void init_sysclock_100 (void)          
{
    // Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
    // The values below will result in a 100Hz counter (100 ticks = 1 second)

    // TCCR0A [ COM0A1 COM0A0 COM0B1 COM0B0 0 0 WGM01 WGM00 ] = 0b00000010
    // TCCR0B [ FOC0A FOC0B 0 0 WGM02 CS02 CS01 CS00 ] = 0b00001100
    // TIMSK0 [ 0 0 0 0  OCIE0B  OCIE0A  TOIE0 0 ] = 0b00000100
    // OCR0A = 0x9a
    // tick = 1/100 second
    // Test using example/millis _delay_ms(1000); = 1000 ticks

    TCCR0A = ( _BV(WGM01) ) ; 
    TCCR0B |= ( _BV(WGM02)  | _BV(CS02) ) ;
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 0x2e;
    sei();
}
#endif

// ****End of Defined Timer Setup Functions****

uint32_t ticks(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        return(ticks_ctr);
    }
    return 0;   
}

