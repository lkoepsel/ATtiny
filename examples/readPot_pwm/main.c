#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>


#define BLUE PB0
#define GREEN PB1
#define WHITE PB2
#define POT PB4
#define TOP 642
#define MID 341
#define HIGH 250
#define MOD 127
#define DIM 32
#define set_PIN(x) (1 << (x))
#define clr_PIN(x) (~(1 << (x)))

volatile uint16_t ADC_result = 0;
volatile uint8_t LED_intensity = 63;
volatile uint8_t LED_pin = 0;


// -------- Functions --------- //
// ISR to set the pin high and the PWM frequency
ISR(TIM0_OVF_vect) 
{
    OCR0A = LED_intensity;
    PORTB |= set_PIN(LED_pin);
}

// ISR to set the pin low, thus the duty cycle of the A_pulse
ISR(TIM0_COMPA_vect) 
{
    PORTB &= clr_PIN(LED_pin);
}

ISR (ADC_vect)      
{
    ADC_result = ADC;                  // read ADC value
}

uint16_t read_ADC(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        return(ADC_result);
    }
    return 0;   
}

static inline void initADC(void) 
{
    ADMUX = 0;  // Clear ADMUX first
    ADMUX |= _BV(MUX1);  // Select ADC2 (PB4), REFS0 = 0, VCC as reference

    // Enable ADC with prescaler /16 (for 9.6MHz clock → 600kHz ADC clock)
    // Enable ADC interrupts
    ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADATE) | _BV(ADPS2);
    sei();
}

static inline void initTC(void) 
{
    // setup clock for hybrid PWM operation
    TCCR0A = 0 ;                                        // Normal operation 
    TCCR0B |= ( _BV(CS00) ) ;                // /8 prescalar (72Hz)
    TIMSK0 |= ( _BV(OCIE0A) | _BV(TOIE0)); // turn on interrupts
    sei();
}

int main(void)
{
    initADC();
    initTC();

    /* set pins to output */
    DDRB |=( _BV(BLUE) | _BV(GREEN) | _BV(WHITE)); 
    // Make sure PB4 is an input (it should be by default)
    DDRB &= ~_BV(POT);
    PORTB |=( _BV(BLUE) | _BV(GREEN) | _BV(WHITE));  // set all high
    _delay_ms(500);

    PORTB &= ~( _BV(BLUE) | _BV(GREEN) | _BV(WHITE));  // set all low
    ADCSRA |= _BV(ADSC);                    // begin ADC conversions

    for (;;)
    {
        volatile uint16_t curr_result = read_ADC();
        volatile uint16_t prev_result = curr_result;

        while (prev_result == curr_result)
        {
            curr_result = read_ADC();
            if (curr_result > TOP) 
            {
                // 642 - 1023 ADC
                PORTB &= ~_BV(GREEN) & ~_BV(WHITE);
                LED_pin = BLUE;
                if (curr_result > (TOP + 220))
                {
                    LED_intensity = HIGH;  
                }
                else if (curr_result > (TOP + 110))
                {
                    LED_intensity = MOD;  
                }
                else
                {
                    LED_intensity = DIM;
                }
            }
            else if (curr_result > MID)
            {
                // 341 - 641 ADC
                PORTB &= ~_BV(BLUE) & ~_BV(WHITE);
                LED_pin = GREEN;  
                if (curr_result > (MID + 220))
                {
                    LED_intensity = HIGH;  
                }
                else if (curr_result > (MID + 110))
                {
                    LED_intensity = MOD;  
                }
                else
                {
                    LED_intensity = DIM;
                }
            }
            else
            {
                // 0 - 340 ADC
                PORTB &= ~_BV(GREEN) & ~_BV(BLUE);
                LED_pin = WHITE;  
                if (curr_result > (220))
                {
                    LED_intensity = HIGH;  
                }
                else if (curr_result > (110))
                {
                    LED_intensity = MOD;  
                }
                else
                {
                    LED_intensity = DIM;
                }
            }
        }
    }
}
