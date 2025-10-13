Here's a simple C99 `itoa` function optimized for the ATtiny13A's limited memory (1KB flash, 64 bytes SRAM):

```c
#include <stdint.h>

char* itoa(int16_t value, char* buffer, uint8_t base) {
    // Only support base 10 for simplicity and size
    if (base != 10) {
        buffer[0] = '\0';
        return buffer;
    }
    
    char* ptr = buffer;
    uint16_t abs_value;
    uint8_t is_negative = 0;
    
    // Handle negative numbers
    if (value < 0) {
        is_negative = 1;
        // Avoid overflow for INT16_MIN
        abs_value = (uint16_t)(0U - value);
    } else {
        abs_value = (uint16_t)value;
    }
    
    // Convert digits in reverse order
    do {
        *ptr++ = '0' + (abs_value % 10);
        abs_value /= 10;
    } while (abs_value);
    
    // Add negative sign if needed
    if (is_negative) {
        *ptr++ = '-';
    }
    
    // Null terminate
    *ptr = '\0';
    
    // Reverse the string
    char* start = buffer;
    char* end = ptr - 1;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    
    return buffer;
}
```

## Key Features

- **C99 Compliant**: Uses standard C99 types (`stdint.h`) [^4]
- **Memory Efficient**: No dynamic allocation, works with caller-provided buffer [^2]
- **Handles INT_MIN**: Uses unsigned arithmetic to avoid overflow issues [^3]
- **Optimized for ATtiny13A**: Uses `int16_t` instead of `int` to save space, and `uint8_t` for base parameter
- **Simple Implementation**: Only supports base 10 to minimize code size

## Usage Example

```c
#include <avr/io.h>

int main(void) {
    char buffer[7];  // Enough for -32768 + null terminator
    int16_t number = -123;
    
    itoa(number, buffer, 10);
    // buffer now contains "-123"
    
    return 0;
}
```

The function reverses the string in-place after building it backwards, avoiding the common reversed output issue [^1]. The buffer size should be at least 7 bytes to handle the full range of `int16_t` values (-32768 to 32767).

[^1]: [c - How do I fix my `itoa` implementation so it... - Stack Overflow](https://stackoverflow.com/questions/56402852/how-do-i-fix-my-itoa-implementation-so-it-doesnt-print-reversed-output) (35%)
[^2]: [c - What is the proper way of implementing a good "itoa()" function?](https://stackoverflow.com/questions/3440726/what-is-the-proper-way-of-implementing-a-good-itoa-function) (25%)
[^3]: [itoa() c implementation int min underflow - Stack Overflow](https://stackoverflow.com/questions/39929982/itoa-c-implementation-int-min-underflow) (22%)
[^4]: [implicit declaration of function itoa is invalid in c99](https://stackoverflow.com/questions/10162465/implicit-declaration-of-function-itoa-is-invalid-in-c99) (18%)
