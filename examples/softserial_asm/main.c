// soft serial - adds a software defined serial port
// Slow serial port, use for non-intensive serial interaction
// Change serial pins in soft_serial.h: SOFT_RX_PIN/SOFT_TX_PIN
// Set baud rate in soft_serial.h: SOFT_BAUD

#include "softserial_asm.h"

int main(void)
{
    init_serial();
    for (;;) {
        uint8_t c = char_read();
        char_write(c);
    }
}
