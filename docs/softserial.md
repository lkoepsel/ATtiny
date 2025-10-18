# Serial Communication Issues with ATtiny13A
## Introduction

### Explanation
This page covers the issues creating a soft serial port on an ATtiny13A. There are several issues:

1. Lack of precision in internal RC oscillator, Arduino Uno R3 uses a 16Mhz ceramic oscillator (*resonator*)
2. Reading at the correct time
3. Use Timer0 instead of software delays for greater accuracy

The solution was to:
1. Use the `osccal` example, to determine the correct *OSCCAL* value for the RC oscillator
2. Setup TIMER0 to provide a precise timing delay
3. Read the bits in the middle of the bit to ensure the greatest accuracy of read.
3. Set the baud rate to 1200 baud, which is whole divider of the processor clock 1.2MHz. This last step worked quite well and given serial communications won't be high volume, it will be *fast enough*.

### What to do
1. Default for *OSCCAL* is 68, set it in *softserial.c:init_soft_serial*
2. Use the default value of 156 defined in *softserial.h:baud_ticks*, this is for 1200 baud

If the above two steps work for both **TX** and **RX**, great. If not, read below to attempt to solve.

## 1. Why Write Works But Read Fails

The write routine might work perfectly while the read routine fails is actually a **classic symptom** of timing issues in software serial. The key difference is in **timing tolerance**:

### Transmit (Write) Operation
- **You control the timing** - your ATtiny13A generates the signal
- The receiving device (your PC/terminal) has sophisticated UART hardware with **automatic baud rate detection** and **sampling oversampling** (typically 16x)
- The receiver can tolerate timing errors up to about ±5% because it resyncs on each start bit

### Receive (Read) Operation  
- You must **sample at precise moments** within each bit period
- Your code samples only **once per bit** at a fixed delay
- **Timing errors accumulate** across all 8 data bits
- By bit 7 or 8, even a 2-3% timing error can cause you to sample in the wrong bit entirely

## Solution

Since your transmit works, your oscillator is probably close but not precise enough for reliable receive. Try this adjusted read routine with **mid-bit sampling**:

```c
int8_t soft_char_read()
{
    int8_t data = 0;

    // Wait for start bit
    while (PINB & (1 << SOFT_RX_PIN)) {} ;

    // Wait 1.5 bit periods to sample first data bit in the middle
    _delay_us(BIT_DURATION + (BIT_DURATION / 2));
    
    // Read each bit
    for (int8_t i = 0; i < 8; i++)
    {
        if (PINB & (1 << SOFT_RX_PIN))
        {
            data |= (1 << i);
        }
        _delay_us(BIT_DURATION);
    }

    // No need to wait for stop bit

    return data;
}
```

## 2. Calibrate the RC Oscillator

The ATtiny13A uses an internal RC oscillator that has significant tolerance - up to ±10% from the factory. [^5] This inaccuracy directly affects the `_delay_us()` function, which relies on the `F_CPU` definition matching the actual oscillator frequency. [^2] When the oscillator runs faster or slower than expected, your `BIT_DURATION` delays become incorrect, causing the sampling points to drift across the incoming serial data stream.

For reliable software serial communication, the timing accuracy needs to be within about ±5% tolerance. [^6] Your corrupted data suggests your oscillator is running outside this acceptable range.


## Solution

To fix this issue, you need to **calibrate the internal oscillator** using the `OSCCAL` register. [^7][^8] The ATtiny13A has separate calibration values for different clock speeds, and proper calibration can improve accuracy to approximately ±2%. [^3]

Here are your options:

1. **Manual OSCCAL Calibration**: Adjust the OSCCAL register value in your code until you receive correct data. Start with small adjustments:
   ```c
   OSCCAL += 5;  // Try increasing or decreasing
   ```

2. **Measure and Calibrate**: Generate a known frequency output on a pin and measure it with an oscilloscope or frequency counter to determine the actual clock speed, then adjust OSCCAL accordingly. [^1]

3. **Fine-tune F_CPU**: If you can't adjust OSCCAL, you might need to redefine `F_CPU` in your makefile to match the actual oscillator frequency rather than the nominal value.

The successful resolution of similar issues confirms that oscillator calibration resolves software UART problems on the ATtiny13A. [^4]

[^1]: [ATtiny Oscillator Calibration : 4 Steps - Instructables](https://www.instructables.com/ATtiny-Oscillator-Calibration/) (22%)     
[^2]: [<util/delay.h>: Convenience functions for busy-wait delay loops](https://www.nongnu.org/avr-libc/user-manual/group__util__delay.html) (16%)      
[^3]: [ATtiny13 - using delay() function - 3rd Party Boards - Arduino Forum](https://forum.arduino.cc/t/attiny13-using-delay-function/480592) (15%)     
[^4]: [Serial Data Attiny13A - Page 2 - Programming - Arduino Forum](https://forum.arduino.cc/t/serial-data-attiny13a/1094674?page=2) (14%)     
[^5]: [ATtiny13 internal oscillator accurate @9.6MHz but inaccurate @4.8 ...](https://forum.arduino.cc/t/attiny13-internal-oscillator-accurate-9-6mhz-but-inaccurate-4-8mhz/674396) (12%)       
[^6]: [How to: Display serial output from a ATTiny13... - Arduino Forum](https://forum.arduino.cc/t/how-to-display-serial-output-from-a-attiny13/645602) (11%)      
[^7]: [ATTiny13a 4.8Mhz serial communication, calibrate... - Arduino Forum](https://forum.arduino.cc/t/attiny13a-4-8mhz-serial-communication-calibrate-osccal/544744) (5%)      
[^8]: [Serial Data Attiny13A - Programming - Arduino Forum](https://forum.arduino.cc/t/serial-data-attiny13a/1094674) (5%)

### OSCCAL Calibration Routine

```c
#include <avr/io.h>
#include <util/delay.h>

// Your existing UART definitions
#define SOFT_TX_PIN PB0
#define SOFT_RX_PIN PB1
#define BIT_DURATION 104  // For 9600 baud

// Your existing write function
void soft_char_write(char data)
{
    // Start bit
    PORTB &= ~(1 << SOFT_TX_PIN);
    _delay_us(BIT_DURATION);

    // Data bits
    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & (1 << i))
        {
            PORTB |= (1 << SOFT_TX_PIN);
        }
        else
        {
            PORTB &= ~(1 << SOFT_TX_PIN);
        }
        _delay_us(BIT_DURATION);
    }

    // Stop bit
    PORTB |= (1 << SOFT_TX_PIN);
    _delay_us(BIT_DURATION);
}

// Send a string
void soft_string_write(const char* str)
{
    while (*str)
    {
        soft_char_write(*str++);
    }
}

// Send a hex byte value
void send_hex_byte(uint8_t value)
{
    uint8_t high = (value >> 4) & 0x0F;
    uint8_t low = value & 0x0F;
    
    soft_char_write(high < 10 ? '0' + high : 'A' + high - 10);
    soft_char_write(low < 10 ? '0' + low : 'A' + low - 10);
}

int main(void)
{
    // Setup TX pin as output, high (idle state)
    DDRB |= (1 << SOFT_TX_PIN);
    PORTB |= (1 << SOFT_TX_PIN);
    
    // Setup RX pin as input with pull-up
    DDRB &= ~(1 << SOFT_RX_PIN);
    PORTB |= (1 << SOFT_RX_PIN);
    
    uint8_t osccal_start = OSCCAL;  // Save factory value
    
    while (1)
    {
        // Test ascending OSCCAL values
        for (uint8_t i = 0; i < 128; i++)
        {
            OSCCAL = osccal_start + i;
            
            // Send current OSCCAL value
            soft_string_write("OSCCAL=");
            send_hex_byte(OSCCAL);
            soft_string_write(": ");
            
            // Send test pattern
            soft_string_write("ABC123\r\n");
            
            _delay_ms(500);  // Delay between tests
            
            // Stop if we reach max value
            if (OSCCAL == 0xFF) break;
        }
        
        // Test descending OSCCAL values
        for (uint8_t i = 1; i < 128; i++)
        {
            if (osccal_start < i) break;  // Don't go negative
            
            OSCCAL = osccal_start - i;
            
            // Send current OSCCAL value
            soft_string_write("OSCCAL=");
            send_hex_byte(OSCCAL);
            soft_string_write(": ");
            
            // Send test pattern
            soft_string_write("ABC123\r\n");
            
            _delay_ms(500);  // Delay between tests
        }
        
        // Return to factory value and pause
        OSCCAL = osccal_start;
        _delay_ms(2000);
    }
}
```

## How to Use

1. **Compile and upload** this code to your ATtiny13A
2. **Connect your serial terminal** at 9600 baud
3. **Watch the output** - you'll see something like:
   ```
   OSCCAL=6A: ABC123
   OSCCAL=6B: ABC123
   OSCCAL=6C: ABC123
   ```
4. **Find the OSCCAL value** where "ABC123" appears correctly without corruption
5. **Note that value** and use it in your main program by adding:
   ```c
   OSCCAL = 0x6C;  // Replace with your optimal value
   ```
   at the beginning of your `main()` function

## Tips

- The factory default OSCCAL value is typically around 0x6A-0x6F [^1]
- The ATtiny13A has separate calibration values for 4.8MHz and 9.6MHz operation [^2][^3]
- Small adjustments (±5) are usually sufficient [^4][^5]
- If all values show corruption, your baud rate might be too far off - try adjusting `BIT_DURATION` slightly

Once you find the correct OSCCAL value, your receive routine should work properly since both transmit and receive will use the same calibrated timing.

[^1]: [ATtiny13A Data Sheet](https://www.farnell.com/datasheets/1714641.pdf) (34%)  
[^2]: [ATTiny13a 4.8Mhz serial communication, calibrate OSCCAL.](https://forum.arduino.cc/t/attiny13a-4-8mhz-serial-communication-calibrate-osccal/544744) (21%)    
[^3]: [ATtiny13 internal oscillator accurate @9.6MHz but inaccurate @4.8 ...](https://forum.arduino.cc/t/attiny13-internal-oscillator-accurate-9-6mhz-but-inaccurate-4-8mhz/674396) (20%)   
[^4]: [ATtiny13 - using delay() function - 3rd Party Boards - Arduino Forum](https://forum.arduino.cc/t/attiny13-using-delay-function/480592) (13%)     
[^5]: [Serial Data Attiny13A - Page 2 - Programming - Arduino Forum](https://forum.arduino.cc/t/serial-data-attiny13a/1094674?page=2) (12%)


## 3. Mid-bit Sampling

The original read routine waits for half a bit period after detecting the start bit, but this isn't quite right. Wait **1.5 bit periods** from the start bit edge to sample in the middle of the first data bit.

## Corrected Read Routine

```c
int8_t soft_char_read()
{
    int8_t data = 0;

    // Wait for start bit (falling edge)
    while (PINB & (1 << SOFT_RX_PIN)) {} ;

    // CRITICAL: Wait 1.5 bit periods to reach middle of first data bit
    // Start bit begins now, we want to sample in middle of bit 0
    _delay_us(BIT_DURATION + (BIT_DURATION / 2));
    
    // Read bit 0
    if (PINB & (1 << SOFT_RX_PIN))
    {
        data |= (1 << 0);
    }
    
    // Read remaining 7 bits
    for (int8_t i = 1; i < 8; i++)
    {
        _delay_us(BIT_DURATION);
        if (PINB & (1 << SOFT_RX_PIN))
        {
            data |= (1 << i);
        }
    }

    // Optional: verify stop bit is present
    _delay_us(BIT_DURATION);
    // Could check for stop bit here if needed

    return data;
}
```

## 4. Timer-Based Sampling

For even better accuracy with your calibrated OSCCAL value of 0x73, you could use Timer0 for precise bit timing:

```c
void timer0_delay_us(uint16_t us)
{
    // Assuming 9.6MHz clock, prescaler /8 = 1.2MHz timer clock
    // Each timer tick = 0.833us
    uint16_t ticks = (us * 12) / 10;  // Approximate conversion
    
    TCCR0B = (1 << CS01);  // Prescaler /8
    
    while (ticks > 0)
    {
        TCNT0 = 0;
        uint8_t wait = (ticks > 255) ? 255 : ticks;
        while (TCNT0 < wait) {} ;
        ticks -= wait;
    }
    
    TCCR0B = 0;  // Stop timer
}
```

The key insight is that software UART receive requires **precise alignment** with the incoming data stream. Your transmit works because the receiving hardware is more tolerant and resyncs on each start bit, but your receive must sample at exactly the right moments.

You're absolutely correct! Let me clarify the clock division chain for the ATtiny13A:

## Clock Division Steps

1. **Internal Oscillator**: 9.6 MHz
2. **System Clock (F_CPU)**: 9.6 MHz ÷ 8 = **1.2 MHz** (due to CKDIV8 fuse)
3. **Timer0 Clock**: 1.2 MHz ÷ 8 = **150 kHz** (with CS01 prescaler)

## Precision Timer Function (16bits, low speed /8)

```c
void timer0_delay_us(uint16_t us)
{
    // System clock: 1.2MHz (9.6MHz / 8)
    // Timer clock with prescaler /8: 150kHz
    // Each timer tick = 6.67us
    
    // Calculate ticks needed (us * 10 / 67 approximates us / 6.67)
    uint16_t ticks = (us * 10) / 67;
    
    TCCR0B = (1 << CS01);  // Prescaler /8
    
    while (ticks > 0)
    {
        TCNT0 = 0;
        uint8_t wait = (ticks > 255) ? 255 : ticks;
        while (TCNT0 < wait) {} ;
        ticks -= wait;
    }
    
    TCCR0B = 0;  // Stop timer
}
```

## Alternative: Use Prescaler /1 for Better Resolution

```c
void timer0_delay_us(uint16_t us)
{
    // Timer clock with prescaler /1: 1.2MHz
    // Each timer tick = 0.833us
    
    // Calculate ticks (us * 6 / 5 approximates us / 0.833)
    uint16_t ticks = (us * 6) / 5;
    
    TCCR0B = (1 << CS00);  // Prescaler /1
    
    while (ticks > 0)
    {
        TCNT0 = 0;
        uint8_t wait = (ticks > 255) ? 255 : ticks;
        while (TCNT0 < wait) {} ;
        ticks -= wait;
    }
    
    TCCR0B = 0;  // Stop timer
}
```

This gives you ~125 ticks for 104µs, providing much better timing resolution for your software UART.

Let me break down this timer delay loop that handles delays longer than what an 8-bit timer can count:

## Timer0 16-bit Function explained 

The ATtiny13A's Timer0 is an **8-bit timer**, meaning `TCNT0` can only count from 0 to 255. If you need a delay of, say, 500 ticks, you can't just wait for `TCNT0` to reach 500 - it physically can't count that high.

## Line-by-Line Explanation

```c
while (ticks > 0)
```
This outer loop continues as long as we have ticks remaining to count. It handles the total delay by breaking it into chunks.

```c
TCNT0 = 0;
```
Reset the timer counter to 0 at the start of each iteration. This ensures we're starting fresh for each chunk of counting.

```c
uint8_t wait = (ticks > 255) ? 255 : ticks;
```
This is the **key line**. It determines how many ticks to count in this iteration:
- If `ticks > 255`: Set `wait = 255` (maximum the 8-bit timer can count)
- If `ticks ≤ 255`: Set `wait = ticks` (count the remaining ticks)

This ternary operator essentially says: "Count either 255 ticks or whatever's left, whichever is smaller."

```c
while (TCNT0 < wait) {} ;
```
This inner loop **busy-waits** until the timer counter reaches our target value. The timer hardware increments `TCNT0` automatically based on the prescaler setting. We just spin here checking if it's reached our target yet.

```c
ticks -= wait;
```
Subtract the ticks we just counted from our total. If we counted 255 ticks and needed 500 total, we now have 245 ticks left for the next iteration.

## Example Walkthrough

For a 500-tick delay:
1. **First iteration**: `wait = 255`, count to 255, `ticks = 245` remaining
2. **Second iteration**: `wait = 245`, count to 245, `ticks = 0` remaining  
3. **Exit loop** (ticks = 0)

This clever approach lets you create delays of any length (up to 65,535 ticks with `uint16_t`) using only an 8-bit timer register.

## Bit Duration Table for Software UART

| Baud Rate | Bit Period | Bit Duration (µs) | Timer Ticks @ 1.2MHz |
|-----------|------------|-------------------|---------------------|
| **1200**  | **1/1200 s**  | **833.33**   | **156 (/8 scalar)**   |
| 2400      | 1/2400 s   | 416.67           | tbd                  |
| 4800      | 1/4800 s   | 208.33           | tbd                  |
| 9600      | 1/9600 s   | 104.17           | 125 (/1 scalar)      |
| 19200     | 1/19200 s  | 52.08            | tbd                  |
| 28800     | 1/28800 s  | 34.72            | tbd                  |

## Important Notes

* **1200 baud works reliably, and at 150 bytes/s, fast enough**
* 9600 baud did not and I wasn't able to find a combination of *OSCCAL* and baud_ticks which delivered ~104.2us reliably
* Other baud rates haven't been tested, will confirm in table when tested

**For Your Code:**
Given your OSCCAL calibration to 0x73, you should define `BIT_DURATION` as:
```c
#define BIT_DURATION 104  // For 9600 baud
// or
#define BIT_DURATION 52   // For 19200 baud
```

The higher baud rates (57600 and 115200) are generally not recommended for software UART on the ATtiny13A running at 1.2MHz due to insufficient CPU cycles to reliably sample and process bits.

[Most Common Baud Rates](https://lucidar.me/en/serialib/most-used-baud-rates-table/)

## Macro for 8-bit Timer

### Basic Macro Definition

```c
#define TIMER_DELAY(ticks) do { \
    TCNT0 = 0; \
    while (TCNT0 < (ticks)) {} ; \
} while(0)
```

## Usage Example

```c
// In your code, you can now use:
TIMER_DELAY(baud_ticks);
// or with a literal value:
TIMER_DELAY(144);
// or with any expression:
TIMER_DELAY(baud_ticks / 2);
```

## Why This Macro Design?

### The `do-while(0)` Wrapper
This is a **critical safety pattern** for multi-statement macros in C. It ensures the macro behaves like a single statement in all contexts:

```c
// Without do-while(0), this would break:
if (condition)
    TIMER_DELAY(baud_ticks);  // Works correctly
else
    something_else();

// The semicolon after the macro call completes the do-while
```

### Parentheses Around `ticks`
The parentheses around `(ticks)` in the comparison protect against operator precedence issues:

```c
// Safe even with expressions:
TIMER_DELAY(baud_ticks + 10);  // Evaluates correctly
```

## Complete Example for Your UART

```c
#include <avr/io.h>
#include "ATtiny.h"

#define TIMER_DELAY(ticks) do { \
    TCNT0 = 0; \
    while (TCNT0 < (ticks)) {} ; \
} while(0)

#define BAUD_9600  144   // Your measured value
#define BAUD_19200 72    // Half the period

int main(void)
{
    DDRB |= (_BV(PORTB0));
    TCCR0B = (1 << CS00);  // Prescaler /1
    OSCCAL = 0x73;

    for (;;)
    {
        SBI(PORTB, PORTB0);
        TIMER_DELAY(BAUD_9600);
        
        CBI(PORTB, PORTB0);
        TIMER_DELAY(BAUD_9600);
    }
}
```

## Alternative: Function-Like Macro with Type Safety

If you want more control, you can add type checking:

```c
#define TIMER_DELAY(ticks) do { \
    uint8_t _ticks = (uint8_t)(ticks); \
    TCNT0 = 0; \
    while (TCNT0 < _ticks) {} ; \
} while(0)
```

This ensures the value is always treated as an 8-bit value, matching the TCNT0 register size.