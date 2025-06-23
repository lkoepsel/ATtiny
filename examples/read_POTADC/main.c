#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

// Pin definitions for LEDs
#define GREEN_LED   PB1
#define YELLOW_LED  PB3
#define BLUE_LED    PB4

// PWM control variables
volatile uint8_t green_brightness = 0;
volatile uint8_t yellow_brightness = 0;
volatile uint8_t blue_brightness = 0;
volatile uint8_t pwm_counter = 0;

// ADC variables
volatile uint16_t ADC_result = 0;

// ADC interrupt - just store the result
ISR(ADC_vect)      
{
    ADC_result = ADC;
}

// Timer0 overflow interrupt - minimal PWM counter increment
ISR(TIM0_OVF_vect) 
{
    pwm_counter++;
}

// Thread-safe ADC result reading
uint16_t read_ADC(void) 
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        return ADC_result;
    }
    return 0;   
}

// ADC initialization for potentiometer on PB2 (ADC1)
static inline void initADC(void) 
{
    // Select ADC1 (PB2), VCC as reference
    ADMUX = _BV(MUX0);  // Select ADC1 (PB2)
    
    // Enable ADC with prescaler /16, enable interrupts and auto-trigger
    ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADATE) | _BV(ADPS2);
    sei();
}

// Timer0 initialization for PWM counter
static inline void initTimer0(void) 
{
    TCCR0A = 0;  // Normal operation 
    TCCR0B |= (_BV(CS01) | _BV(CS00));  // Prescaler /64
    TIMSK0 |= _BV(TOIE0);  // Enable overflow interrupt
}

// Software PWM update function
void updatePWM(void)
{
    // Control Green LED (PB1)
    if (pwm_counter < green_brightness) {
        PORTB |= _BV(GREEN_LED);
    } else {
        PORTB &= ~_BV(GREEN_LED);
    }
    
    // Control Yellow LED (PB3)
    if (pwm_counter < yellow_brightness) {
        PORTB |= _BV(YELLOW_LED);
    } else {
        PORTB &= ~_BV(YELLOW_LED);
    }
    
    // Control Blue LED (PB4)
    if (pwm_counter < blue_brightness) {
        PORTB |= _BV(BLUE_LED);
    } else {
        PORTB &= ~_BV(BLUE_LED);
    }
}

// Map ADC value to LED selection and brightness
void updateLEDs(uint16_t adc_value)
{
    // Reset all LED brightness values
    green_brightness = 0;
    yellow_brightness = 0;
    blue_brightness = 0;
    
    // ADC range is 0-1023, divide into 3 regions for 3 LEDs
    // Each LED gets ~341 steps (1024/3)
    
    if (adc_value < 341) {
        // Green LED region (0-340)
        // Map 0-340 to brightness 0-255
        green_brightness = (uint8_t)((adc_value * 255UL) / 340);
    }
    else if (adc_value < 682) {
        // Yellow LED region (341-681)
        // Map 341-681 to brightness 0-255
        uint16_t adjusted_value = adc_value - 341;
        yellow_brightness = (uint8_t)((adjusted_value * 255UL) / 340);
    }
    else {
        // Blue LED region (682-1023)
        // Map 682-1023 to brightness 0-255
        uint16_t adjusted_value = adc_value - 682;
        blue_brightness = (uint8_t)((adjusted_value * 255UL) / 341);
    }
}

int main(void)
{
    // Initialize ADC
    initADC();
    
    // Set LED pins as outputs
    DDRB |= _BV(GREEN_LED) | _BV(YELLOW_LED) | _BV(BLUE_LED);
    
    // Ensure PB2 (potentiometer) is input
    DDRB &= ~_BV(PB2);
    
    // Initialize Timer0 for PWM counter
    initTimer0();
    
    // Start first ADC conversion
    ADCSRA |= _BV(ADSC);
    
    // Main loop
    for (;;)
    {
        // Read potentiometer value using interrupt-based ADC
        uint16_t pot_value = read_ADC();
        
        // Update LED brightness based on potentiometer
        updateLEDs(pot_value);
        
        // Update PWM outputs
        updatePWM();
        
        // Small delay to prevent excessive processing
        _delay_ms(10);
    }
    
    return 0;
}
