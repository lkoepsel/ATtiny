// soft serial - adds a software defined serial port
// Slow serial port, use for non-intensive serial interaction
// Change serial pins in soft_serial.h: SOFT_RX_PIN/SOFT_TX_PIN
// Set baud rate in soft_serial.h: SOFT_BAUD

#include <stdio.h>
#include "soft_serial.h"

const char prompt[] PROGMEM = "13A:";
const char waiting[] PROGMEM = "W:";
const char chr_recd[] PROGMEM = " R:";

#define N_in 7
#define N_out 7

int main(void) {
    char soft_in[N_in] = {""};
    char soft_out[N_out] = {""};

    // Example: Send and receive data
    init_soft_serial();
    soft_char_write(CR);
    soft_char_write(LF);
    soft_pgmtext_write(prompt);
    soft_char_write(BL);
    soft_pgmtext_write(waiting);

    while (1) 
    {
        // Receive data
        uint8_t received = soft_readLine(soft_in, N_in - 1);
        soft_pgmtext_write(chr_recd);
        soft_char_write(received + ASCII_INTEGER);
        soft_char_write(BL);
        soft_string_write(soft_out, (sizeof(soft_out)/sizeof(soft_out[0])));
        soft_string_write(soft_in, received + 1);
        soft_char_write(CR);
        soft_char_write(LF);
    }

    return 0;
}
