//  blink_spwm - uses bit setting by asm commands, and creates a soft PWM

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t TOP = 255;
volatile uint8_t pulse = 255;
#define GREEN PB0
#define YELLOW PB1
#define BLUE PB2


ISR (TIM0_COMPA_vect)      
{
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PINB)), "I" (GREEN));
}

// ****Defined Timer Setup Functions****
void init_pulse (void)          
{
    // Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
    // The values below will result in a 1kHz counter (1000 ticks = 1 second)
    // CTC mode (WGM0[2:0] = 2)
    // Set clock select to /8 CS => 010
    // Bit 2 – OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
    // OCR0A = x9c or ~150

    // TCCR0A [ COM0A1 COM0A0 COM0B1 COM0B0 0 0 WGM01 WGM00 ] = 0b00000010
    // TCCR0B [ FOC0A FOC0B 0 0 WGM02 CS02 CS01 CS00 ] = 0b00000010
    // TIMSK0 [ 0 0 0 0  OCIE0B  OCIE0A  TOIE0 0 ] = 0b00000100
    // OCR0A = 0x9a
    // tick = 1/1000 second
    // Test using example/ticks w/ _delay_ms(1000); = 1000 ticks

    TCCR0A = ( _BV(WGM01) ) ; 
    TCCR0B |= ( _BV(CS01) ) ;
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = pulse;
    sei();
}

int main(void)
{
    /* set pin to output*/
    DDRB |= (_BV(GREEN));
    DDRB &= ~(_BV(BLUE)) & ~(_BV(YELLOW));
    init_pulse();
    while (1){}
    return 0; 
}


