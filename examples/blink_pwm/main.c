// blink demo of how to get PWM on any pin with interrupts
// based on code by Mike Williams, Make AVR book
// uses asm commmands where possible to save memory

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#define GREEN PB0
#define YELLOW PB1

volatile uint8_t A_pulse = 32;
volatile uint8_t B_pulse = 63;

ISR(TIM0_OVF_vect) 
{
    OCR0A = A_pulse;
    OCR0B = B_pulse;
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
}
ISR(TIM0_COMPA_vect) 
{
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
}
ISR(TIM0_COMPB_vect) 
{
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
}

int main(void) 
{
    // setup clock for hybrid PWM operation
    TCCR0A = 0 ;                                        // Normal operation 
    TCCR0B |= ( _BV(CS01) | _BV(CS00)) ;                // /64 prescalar (72Hz)
    TIMSK0 |= (_BV(OCIE0B) | _BV(OCIE0A) | _BV(TOIE0)); // turn on all interrupts
    sei();

    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (GREEN));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (YELLOW));

    while (1)  // loop can be used to adjust duty cycle of two pwm signals
    {
        _NOP();
    }                                         
  return 0;                            // This line is never reached 
}
