You can create a macro to simplify your inline assembly ```sbi``` command. Here's how to do it:

## Inline Assembly Macro

```c
#define SBI(port, bit) \
    __asm__ __volatile__ ( \
        "sbi %0, %1" \
        : \
        : "I" (_SFR_IO_ADDR(port)), \
          "I" (bit) \
    )
```

## Usage Example

```c
#include <avr/io.h>

#define LED 3  // PB3

int main(void) {
    // Set PB3 as output using your macro
    SBI(DDRB, LED);
    
    // Turn on LED
    SBI(PORTB, LED);
    
    while(1) {
        // Main loop
    }
}
```

## Important Notes

- The ```__volatile__``` keyword ensures the compiler won't optimize away the assembly instruction [^1][^2]
- The ```"I"``` constraint specifies an immediate value in the range 0-63, which is required for the ```sbi``` instruction
- ```_SFR_IO_ADDR()``` converts the memory-mapped I/O address to the I/O space address needed by ```sbi```

## Alternative: AVR-LibC Idiomatic Approach

Since avr-libc 2.2 no longer directly supports ```sbi()``` and ```cbi()``` macros, the recommended approach is to use standard C bit manipulation [^3]:

```c
// Instead of sbi(DDRB, LED):
DDRB |= _BV(LED);

// Instead of cbi(DDRB, LED):
DDRB &= ~_BV(LED);
```

The inline assembly macro will generate slightly smaller code (single ```sbi``` instruction), while the C bit manipulation approach gives the compiler more optimization opportunities and is more portable. For the ATtiny13A with its limited flash memory, the inline assembly macro can save a few bytes per operation.

[^1]: [Inline Assembler Cookbook](https://avr-libc.nongnu.org/user-manual/inline_asm.html#:~:text=AVR-GCC%20Inline,port%20D)
[^2]: [Inline Assembler Cookbook - AVR-LibC - GitHub Pages](https://avrdudes.github.io/avr-libc/avr-libc-user-manual-2.2.0/inline_asm.html#:~:text=A%20GCC,ANSI%20mode.)
[^3]: [avr-libc - Special function registers - Savannah.nongnu.org](https://www.nongnu.org/avr-libc/user-manual/group__avr__sfr.html#:~:text=avr-libc%202.1.0.The,_BV%28bit%29%20.)