// blink_pwm - blinks by setting up a PWM via interrupts
// based on code by Mike Williams, Make AVR book
// uses asm commands where possible to save memory

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#define GREEN PB0
#define YELLOW PB1

volatile uint8_t A_pulse = 63;
volatile uint8_t B_pulse = 127;

// ISR to set the pin high and the PWM frequency
ISR(TIM0_OVF_vect) 
{
    OCR0A = A_pulse;
    OCR0B = B_pulse;
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (YELLOW));
}

// ISR to set the pin low, thus the duty cycle of the A_pulse
ISR(TIM0_COMPA_vect) 
{
    asm ("cbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (GREEN));
}

// ISR to set the pin low, thus the duty cycle of the B_pulse
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

    // set both pins to be outputs
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (GREEN));
    asm ("sbi %0, %1 \n" : : "I" (_SFR_IO_ADDR(DDRB)), "I" (YELLOW));

    // loop can be used to adjust duty cycle of two pwm signals
    while (1)
    {
        _NOP();
    }                                         
  return 0;                            // This line is never reached 
}
