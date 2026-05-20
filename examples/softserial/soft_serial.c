#include <util/delay.h>
#include "soft_serial.h"

char num_string[6] = {};

void soft_int16_write(int16_t number)
{
    itoa(number, num_string, 10);
    soft_string_write(num_string);
}

void soft_uint16_write(uint16_t n)
{
    char buf[6];
    itoa(n, buf, 10);
    soft_string_write(buf);
}

void soft_string_write(const char *s)
{
    while (*s) char_write((uint8_t)*s++);
}

int8_t soft_readLine(char *buffer, int8_t SIZE)
{
    int8_t n_chars = 0;
    int8_t EOL = 0;
    do
    {
        char temp = char_read();
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
        char_write(c);
    }
}
