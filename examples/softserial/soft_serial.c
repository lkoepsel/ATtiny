#include "soft_serial.h"

char num_string[6] = {};

void init_soft_serial()
{
    // // Set TX pin as output, set RX pin as input, RX as input pullup
    DDRB |= _BV(SOFT_TX_PIN);
    DDRB &= ~_BV(SOFT_RX_PIN);
    PORTB |= _BV(SOFT_RX_PIN);
    OSCCAL = 0x73; // use ../osscal routine to determine optimal value
    TCCR0B = (1 << CS01);  // Prescaler /8

}

void soft_char_write(char data)
{
    // Start bit
    PORTB &= ~(1 << SOFT_TX_PIN);
    TIMER_DELAY(baud_ticks);

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
        TIMER_DELAY(baud_ticks);
    }

    // Stop bit
    PORTB |= (1 << SOFT_TX_PIN);
    TIMER_DELAY(baud_ticks);

}

// read routine, waits til middle of bit to check value
int8_t soft_char_read()
{
    int8_t data = 0;

    // Wait for start bit
    while (PINB & (1 << SOFT_RX_PIN)) {} ;

    // Wait 1.5 bit periods to sample first data bit in the middle
    TIMER_DELAY(baud_ticks + baud_ticks / 2);
    
    // Read each bit
    for (int8_t i = 0; i < 8; i++)
    {
        if (PINB & (1 << SOFT_RX_PIN))
        {
            data |= (1 << i);
        }
        TIMER_DELAY(baud_ticks);
    }

    return data;
}

void soft_int16_write(int16_t number)
{
    itoa(number, num_string, 10);
    soft_string_write(num_string, 6);
}

void soft_uint16_write(uint16_t number)
{
    itoa(number, num_string, 10);
    soft_string_write(num_string, 5);
}

int8_t soft_string_write(char *buffer, int8_t len)
{
    // Transmit data
    int8_t count = 0;
    while ((*buffer != '\0') && (count++ <= len))
    {
        soft_char_write(*buffer++);
    }

    return count;
}

int8_t soft_readLine(char *buffer, int8_t SIZE)
{
    int8_t n_chars = 0;
    int8_t EOL = 0;
    do
    {
        char temp = soft_char_read();
        if (temp == CR)
        {
            EOL = 1;
        }
        else
        {
            buffer[n_chars++] = temp;
            if (n_chars >= SIZE)
            {
                EOL = 1;
            }
        }
    } while (!EOL);
    buffer[n_chars] = 0;
    return n_chars;
}

void soft_pgmtext_write(const char *pgm_text)
{
    for (uint8_t i = 0; i < strlen_P(pgm_text); i++)
    {
        uint8_t c = pgm_read_byte(&(pgm_text[i]));
        soft_char_write(c);
    }
}
