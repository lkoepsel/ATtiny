#include <avr/io.h>
#include <util/delay.h>

// Your existing UART definitions
#define SOFT_TX_PIN PB3
#define SOFT_RX_PIN PB4
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
