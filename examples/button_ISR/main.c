/* button - Demonstrates how to add buttons and 
*  test software reset with a user-defined button.
* The button must attach to a pin, setup as INPUT_PULLUP and will be debounced
* Requires sysclock_2:
*       #include "sysclock.h"
*       init_sysclock_2()
*
*       buttons is a struct with elements
*           pin - pin pin for button
*           pressed - if true, button has been pressed
*       soft reset is setup by init_RESET()
*       button is assumed to be on PB7, INPUT_PULLUP and will be debounced
*       reset is performed by a WDT interrupt as described by AVR-GCC FAQ  
*/ 
#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include "ATtiny.h"

#define BOUNCE_DIVIDER 32 // divides millis by this number for checking reset button
#define BOUNCE_MASK 0b11000111
#define BUTTON PB3

volatile uint32_t sys_ctr_2 = 0;
volatile uint8_t button = PB3;
volatile uint8_t bounce_delay = BOUNCE_DIVIDER;
volatile bool pressed = false;    
volatile uint8_t count = 0;

 bool is_button_pressed(void);

// ****End of Defined Interrupt Service Routines****

// ****Defined Timer Setup Functions****

volatile uint8_t ticks_ctr = 0;

// Incrments ticks_ctr for measuring time
ISR (TIM0_COMPA_vect)      
{
    ticks_ctr++;

    //  X times divider for millis() otherwise buttons checked too often
    bounce_delay--;
    if (bounce_delay == 0) 
    {
        pressed = is_button_pressed();
        bounce_delay = BOUNCE_DIVIDER;
    }
    // DEBUG: Use to check frequency, currently 200Hz (1/(25ms * 2))
    // SBI(PINB, PB3);
}

// Initialize timer to ~40 ticks for 1 seconds (1 tick = 25ms or max = ~6.5)
// Initialize timer 0 to CTC Mode using OCR0A, with a chip clock of 1.2Mhz
// The values below will result in a ~62.5Hz counter (40 ticks = 1 second)
// CTC mode (WGM0[2:0] = 2)
// Set clock select to /256 CS => 100
// Bit 2 – OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
// OCR0A = x75
// TC0 Register Values, as working
// "GTCCR", 0x00000048, 0x00 (0, 0b00000000), PSR10: 0b0, TSM: 0b0
// "OCR0B", 0x00000049, 0x00 (0, 0b00000000)
// "TCCR0A", 0x0000004F, 0x02 (2, 0b00000010), WGM0: 0b10, COM0B: 0b00, COM0A: 0b00
// "TCNT0", 0x00000052, 0x4C (76, 0b01001100)
// "TCCR0B", 0x00000053, 0x04 (4, 0b00000100), CS0: 0b100, WGM02: 0b0, FOC0B: 0b0, FOC0A: 0b0
// "OCR0A", 0x00000056, 0x75 (117, 0b01110101)
// "TIFR0", 0x00000058, 0x08 (8, 0b00001000), TOV0: 0b0, OCF0A: 0b0, OCF0B: 0b1
// "TIMSK0", 0x00000059, 0x04 (4, 0b00000100), TOIE0: 0b0, OCIE0A: 0b1, OCIE0B: 0b0
void init_sysclock (void)          
{
    TCCR0A = ( _BV(WGM01) ) ; 
    TCCR0B |= ( _BV(CS02) ) ;
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = 0x75;
    sei();
 }

 bool is_button_pressed(void){

    static uint8_t button_history;
 
    button_history = button_history << 1;
    button_history |= ((PINB & (1 << button)) == 0);

    if ((button_history & BOUNCE_MASK) == 0b00000111){ 
        pressed = true;
        button_history = 0b11111111;
    }
    return pressed;
}


int main (void)
{
    // button => INPUT_PULLUP;
    CBI(DDRB, BUTTON);
    SBI(PORTB, BUTTON);

    init_sysclock ();

    /* loop forever, the interrupts are doing the rest */
    for (;;)  
    {
        for (uint8_t i=0; i < 10; i++) 
        {
            if (pressed) 
            {
                count += 1;
            }
        }
    }
}

