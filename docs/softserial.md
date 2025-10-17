Looking at your received data pattern, you're experiencing bit shifting and corruption typical of timing issues in software UART implementations on the ATtiny13A.

## The Problem

Your received values show a consistent pattern of corruption:
- '1' (0x31) → 0xC3
- '2' (0x32) → 0xC4  
- '3' (0x33) → 0x67

This indicates your bit sampling is happening at incorrect times, causing you to read bits at the wrong positions within each transmitted byte.

## Root Cause

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

You're absolutely right to question this! The fact that your write routine works perfectly while your read routine fails is actually a **classic symptom** of timing issues in software serial, but not for the reason you might think.

## Why Write Works But Read Fails

The key difference is in **timing tolerance**:

### Transmit (Write) Operation
- **You control the timing** - your ATtiny13A generates the signal
- The receiving device (your PC/terminal) has sophisticated UART hardware with **automatic baud rate detection** and **sampling oversampling** (typically 16x)
- The receiver can tolerate timing errors up to about ±5% because it resyncs on each start bit

### Receive (Read) Operation  
- You must **sample at precise moments** within each bit period
- Your code samples only **once per bit** at a fixed delay
- **Timing errors accumulate** across all 8 data bits
- By bit 7 or 8, even a 2-3% timing error can cause you to sample in the wrong bit entirely

## The Real Problem

Looking at your corrupted data pattern more closely:
- '1' (0x31 = 0011 0001) → 0xC3 (1100 0011)
- '2' (0x32 = 0011 0010) → 0xC4 (1100 0100)
- '3' (0x33 = 0011 0011) → 0x67 (0110 0111)

This shows your sampling is drifting across bit boundaries. You're likely sampling **too early**, causing bits to shift and corrupt.

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

If this doesn't work, you'll still need to calibrate OSCCAL, but only by a small amount since your transmit already works.

Here's a simple OSCCAL calibration routine for your ATtiny13A that transmits test characters so you can find the optimal value:

## OSCCAL Calibration Routine

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


Excellent! The OSCCAL calibration worked, but now you're seeing a different pattern. Looking at your results:

- '1' (0x31) → 'a' (0x61)
- '2' (0x32) → 'b' (0x62)  
- '3' (0x33) → 'c' (0x63)
- '4' (0x34) → 'h' (0x68)
- '5' (0x35) → 'm' (0x6D)

This pattern shows **bit 5 is being set incorrectly** (adding 0x30 to each character). This is a classic symptom of sampling timing drift within the byte.

## The Problem

Your original read routine waits for half a bit period after detecting the start bit, but this isn't quite right. You need to wait **1.5 bit periods** from the start bit edge to sample in the middle of the first data bit.

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

## Alternative: Simplified Version

```c
int8_t soft_char_read()
{
    int8_t data = 0;

    // Wait for start bit
    while (PINB & (1 << SOFT_RX_PIN)) {} ;

    // Skip the start bit completely
    _delay_us(BIT_DURATION);
    
    // Now sample each data bit in its middle
    for (int8_t i = 0; i < 8; i++)
    {
        _delay_us(BIT_DURATION / 2);  // Move to middle of bit
        
        if (PINB & (1 << SOFT_RX_PIN))
        {
            data |= (1 << i);
        }
        
        _delay_us(BIT_DURATION / 2);  // Complete the bit period
    }

    return data;
}
```

The key insight is that your original code was sampling too early in each bit period, causing cumulative drift that corrupted higher-order bits. With OSCCAL calibrated to 0x73, these corrected timing routines should give you accurate reception.

Since your transmit works perfectly but receive fails with the same consistent pattern, this points to a very specific issue with **bit sampling alignment** in your software UART implementation.

## The Real Problem: Start Bit Detection Edge

Looking at your corrupted data more carefully:
- '1' (0x31 = 00110001) → 'a' (0x61 = 01100001)  
- '2' (0x32 = 00110010) → 'b' (0x62 = 01100010)
- '3' (0x33 = 00110011) → 'c' (0x63 = 01100011)

The pattern shows you're **reading one bit position too late** - you're getting bit 7 of the previous byte as bit 0 of your current byte. This is a classic framing error.

## The Issue

Your code waits for the start bit edge but then only delays half a bit period. This puts you at the **edge** of the start bit, not in the middle of the first data bit. The subsequent reads drift across bit boundaries.

## Solution: Proper Bit-Banged UART Timing

Here's the corrected receive routine with proper timing:

```c
int8_t soft_char_read()
{
    int8_t data = 0;
    uint8_t bit_count;

    // Wait for idle state first (line high)
    while (!(PINB & (1 << SOFT_RX_PIN))) {} ;
    
    // Wait for start bit (falling edge)
    while (PINB & (1 << SOFT_RX_PIN)) {} ;

    // Critical: Skip past the entire start bit
    _delay_us(BIT_DURATION);
    
    // Now we're at the beginning of bit 0
    // Sample in the middle of each data bit
    for (bit_count = 0; bit_count < 8; bit_count++)
    {
        _delay_us(BIT_DURATION / 2);  // Move to middle of bit
        
        if (PINB & (1 << SOFT_RX_PIN))
        {
            data |= (1 << bit_count);
        }
        
        _delay_us(BIT_DURATION / 2);  // Complete the bit period
    }

    // Optional: Check stop bit
    _delay_us(BIT_DURATION / 2);
    // Stop bit should be high here

    return data;
}
```

## Alternative: Timer-Based Sampling

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

## Corrected Timer Function

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

For your 104µs bit duration at 9600 baud:
- 104µs ÷ 6.67µs = ~15.6 ticks
- The function would use 15 or 16 timer ticks

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

## The Problem This Solves

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

Here's a table showing the bit durations for common baud rates used with software UART on the ATtiny13A:

## Bit Duration Table for Software UART

| Baud Rate | Bit Period | Bit Duration (µs) | Timer Ticks @ 1.2MHz |
|-----------|------------|-------------------|---------------------|
| 9600      | 1/9600 s   | 104.17           | 125                 |
| 19200     | 1/19200 s  | 52.08            | 63                  |
| 28800     | 1/28800 s  | 34.72            | 42                  |
| 38400     | 1/38400 s  | 26.04            | 31                  |
| 57600     | 1/57600 s  | 17.36            | 21                  |
| 115200    | 1/115200 s | 8.68             | 10                  |

## Important Notes

**Timing Accuracy Requirements:**
- Software UART requires timing accuracy within ±5% for reliable communication
- Higher baud rates are more sensitive to timing errors
- At 115200 baud with only 10 timer ticks per bit, you have very little margin for error

**Practical Limitations on ATtiny13A:**
- **9600 baud**: Most reliable, good tolerance for timing variations
- **19200 baud**: Still practical with careful timing
- **38400 baud and above**: Increasingly difficult due to:
  - Limited processing time between bits
  - Cumulative timing errors
  - Interrupt latency issues if using interrupts

**For Your Code:**
Given your OSCCAL calibration to 0x73, you should define `BIT_DURATION` as:
```c
#define BIT_DURATION 104  // For 9600 baud
// or
#define BIT_DURATION 52   // For 19200 baud
```

The higher baud rates (57600 and 115200) are generally not recommended for software UART on the ATtiny13A running at 1.2MHz due to insufficient CPU cycles to reliably sample and process bits.

[Most Common Baud Rates](https://lucidar.me/en/serialib/most-used-baud-rates-table/)

Here's how to create a macro for that timer delay in avr-gcc:

## Basic Macro Definition

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