// One line kernal for multitasking
// https://www.embedded.com/a-multitasking-kernel-in-one-line-of-code-almost/
// Designed to demonstrate the 13A performing different tasks
// with different delays, using a single function pointer array.

#include <avr/io.h>
#include <util/delay.h>

#define NTASKS 5

// Uno pin numbers
enum {LED0, LED1, LED2, LED3, LED4};
volatile uint8_t MS0=255;
volatile uint8_t MS1=100;
volatile uint8_t MS2=150;
volatile uint8_t MS3=200;
volatile uint8_t MS4=250;


void zero (void) {
    /* toggle led on and off */
    PINB |= _BV(LED0);
    _delay_ms(255);
    return;
} 

void one (void) {
    /* toggle led on and off */
    PINB |= _BV(LED1);
    _delay_ms(100);
    return;
} 

void two (void) {
    /* toggle led on and off */
    PINB |= _BV(LED2);
    _delay_ms(150);
    return;
} 

void three (void) {
    /* toggle led on and off */
    PINB |= _BV(LED3);
    _delay_ms(200);
    return;
} 

void four (void) {
    /* toggle led on and off */
    PINB |= _BV(LED4);
    _delay_ms(50);
    return;
} 

void (*tasklist[NTASKS])() = {zero, one, two, three, four};

int main(void)
{
    DDRB |= (_BV(LED0) | _BV(LED1) | _BV(LED2) | _BV(LED3) | _BV(LED4));

    while (1)
    {
    for (uint8_t taskcount=0; taskcount < NTASKS; ++taskcount)
        {
            (*tasklist[taskcount])();

        }
    }
    return 0; 
}

