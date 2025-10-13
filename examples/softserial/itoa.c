#include <stdint.h>

char* itoa(int16_t value, char* buffer) {
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
