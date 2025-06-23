# PWM and ADC Control Explanation for ATtiny13A

## Overall Code Architecture

The code implements a **software PWM (Pulse Width Modulation)** system that controls three LEDs based on a potentiometer input. Here's how each component works:

## 1. Timer0 Interrupt Service Routine (ISR)

```c
ISR(TIM0_OVF_vect) 
{
    pwm_counter++;
}
```

This extremely simple ISR is the heart of the PWM system:

- **Timer0** is configured with a `/64` prescaler, so it overflows at regular intervals
- Every time Timer0 overflows, this ISR executes and increments `pwm_counter`
- With a 9.6MHz clock and `/64` prescaler, Timer0 runs at 150kHz (9.6MHz/64)
- Timer0 is 8-bit, so it overflows every 256 counts, giving us an overflow frequency of ~586Hz (150kHz/256)
- This means `pwm_counter` increments ~586 times per second

## 2. How PWM Counter Creates Variable Intensity

The magic happens in the `updatePWM()` function:

```c
void updatePWM(void)
{
    // Control Green LED (PB1)
    if (pwm_counter < green_brightness) {
        PORTB |= _BV(GREEN_LED);    // Turn LED ON
    } else {
        PORTB &= ~_BV(GREEN_LED);   // Turn LED OFF
    }
    // ... similar for other LEDs
}
```

Here's the detailed PWM mechanism:

### PWM Counter Cycle
- `pwm_counter` is an 8-bit variable that automatically wraps from 255 back to 0
- This creates a repeating sawtooth pattern: 0, 1, 2, 3, ... 254, 255, 0, 1, 2, ...
- Each complete cycle (0→255) takes 256 timer overflows = 256/586Hz ≈ **437ms per PWM cycle**

### Brightness Control Logic
For each LED, the brightness value (0-255) determines the **duty cycle**:

- **brightness = 0**: LED is always OFF
  - `pwm_counter` is never < 0, so LED stays OFF
  
- **brightness = 127** (50% duty cycle): LED is ON half the time
  - When `pwm_counter` = 0-126: LED is ON (127 out of 256 timer ticks)
  - When `pwm_counter` = 127-255: LED is OFF (129 out of 256 timer ticks)
  
- **brightness = 255**: LED is always ON
  - `pwm_counter` is always < 255 (except for 1 timer tick), so LED stays ON ~99.6% of the time

### Visual Example of PWM Waveform

```
brightness = 64 (25% duty cycle):

pwm_counter:  0  32  64  96 128 160 192 224   0  32  64  96 128 160 192 224
LED state:   ON  ON  ON  OFF OFF OFF OFF OFF  ON  ON  ON  OFF OFF OFF OFF OFF
             |----25%----|-----75%-----|     |----25%----|-----75%-----|
             ^                                ^
             PWM cycle repeats every 256 timer overflows
```

## 3. ADC Interrupt System

```c
ISR(ADC_vect)      
{
    ADC_result = ADC;  // Store ADC reading
}
```

- The ADC is configured for **auto-trigger mode** with interrupts enabled
- Every time an ADC conversion completes, this ISR stores the result
- The main loop reads this value safely using atomic blocks

## 4. Potentiometer to LED Mapping

The `updateLEDs()` function divides the 10-bit ADC range (0-1023) into three regions:

```c
if (adc_value < 341) {
    // Green LED: 0-340 maps to brightness 0-255
    green_brightness = (adc_value * 255) / 340;
}
else if (adc_value < 682) {
    // Yellow LED: 341-681 maps to brightness 0-255
    yellow_brightness = ((adc_value - 341) * 255) / 340;
}
else {
    // Blue LED: 682-1023 maps to brightness 0-255
    blue_brightness = ((adc_value - 682) * 255) / 341;
}
```

## 5. Main Loop Operation

```c
for (;;)
{
    uint16_t pot_value = read_ADC();  // Get latest ADC reading
    updateLEDs(pot_value);            // Calculate LED brightness values
    updatePWM();                      // Apply PWM to LED outputs
    _delay_ms(10);                    // Small delay
}
```

The main loop:
1. **Reads** the latest potentiometer value from the ADC interrupt
2. **Calculates** which LED should be active and its brightness
3. **Applies** the PWM by comparing counters to brightness values
4. **Repeats** every 10ms

## 6. Why This Creates Smooth Intensity Changes

The human eye perceives the rapid ON/OFF switching as a steady intensity because:

- **PWM frequency** (~586Hz) is much faster than human vision can detect
- **Duty cycle** (percentage of time LED is ON) directly controls perceived brightness
- **Smooth transitions** occur as the potentiometer gradually changes the duty cycle

## Summary

The `pwm_counter` increment in the ISR creates a timebase that, when compared against brightness values in the main loop, generates precise duty cycles for each LED. As you turn the potentiometer:

1. ADC value changes
2. Brightness calculation maps this to 0-255 range
3. PWM comparison creates corresponding duty cycle
4. LED appears to smoothly dim/brighten
5. At region boundaries, the next LED begins to illuminate

This creates the effect of one LED dimming to off while the next LED brightens from dim to full intensity as you rotate the potentiometer.
