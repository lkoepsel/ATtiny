# Software Serial on the ATtiny13A

To test this software, you must have 1) a serial cable and 2) a serial program. 

## 1. Serial Cable
You will also need a *USB to serial* cable, which converts USB to signals from the ATtiny13A. There are many out there, I've used this one from [Adafruit](https://www.adafruit.com/product/954?gad_source=1) with great success.

The connections would be:
* Green TX -> PB01 Receive pin (**RX**) (*Confirm these port numbers in Library/registers*)
* White RX -> PB02 Transmit pin (**TX**) (*Confirm these port numbers in Library/registers*)
* Black Ground - Ground rail or pin 
* RED Power - leave disconnected

Be sure to use `tio -l` (*below*) to identify the correct serial port for communications. For example, in the example below, */dev/ttyUSB0* is the port for *tio*, while the other port is used by *bloom* to communicate with the *SNAP*:

```bash
Device            TID     Uptime [s] Driver           Description
----------------- ---- ------------- ---------------- --------------------------
/dev/ttyUSB0      khhE       526.385 cp210x           CP2102 USB to UART Bridge Controller
/dev/ttyACM0      D4kU       256.721 cdc_acm          USB2.0 Hub
```

## 2. Serial program
The one I believe works the best in a CLI environment, is [**tio**](https://github.com/tio/tio). 

### tio configuration

For *tio* to work properly, it must have a *.tioconfig* file in your home folder. This allows you to `tio acm` if you wish to use the */dev/ttyACM0* port to connect with an ATmega328P running at 250,000 baud. In this case (*ATtiny13A*), there are two configurations, cna and cnh. The former is for ASCII communications at 9600 baud, while the latter is for binary.

Copy the text below and paste into the file *~/.tioconfig*:

```bash
[default]
baudrate = 250000
databits = 8
parity = none
stopbits = 1
local-echo = false
color = 1

[acm]
device = /dev/ttyACM0
color = 12

[usb]
device = /dev/ttyUSB0
color = 3

[usb-devices]
pattern = ^usb([0-9]*)
device = /dev/ttyUSB%m1
color = 3

[cna]
device = /dev/ttyACM0
color = 12
baudrate = 9600

[cnh]
device = /dev/ttyACM0
color = 12
baudrate = 9600
output-mode = hex
```

Test using the command `tio -l`, you will get something like:

```bash
$ tio -l
Device            TID     Uptime [s] Driver           Description
----------------- ---- ------------- ---------------- --------------------------
/dev/ttyACM0      C4kU      1633.534 cdc_acm          USB2.0 Hub

By-id
--------------------------------------------------------------------------------
/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_7513533363635141E001-if00

By-path
--------------------------------------------------------------------------------
/dev/serial/by-path/platform-fd500000.pcie-pci-0000:01:00.0-usbv2-0:1.2:1.0
/dev/serial/by-path/platform-fd500000.pcie-pci-0000:01:00.0-usb-0:1.2:1.0

Configuration profiles (/home/lkoepsel/.tioconfig)
--------------------------------------------------------------------------------
acm                 usb                 usb-devices         cna
cnh
```

From this you can see the serial port name */dev/ttyACM0* along with the possible configuration profiles.

## Status

The software serial port is now implemented in **AVR assembly**
(`Library/serial.S`), exposed to C through `Library/serial_asm.h`, and runs
**rock-solid at 9600 baud** (8-N-1) on the 1.2 MHz internal RC oscillator. The
earlier pure-C approaches on this page (busy-wait `_delay_us`, Timer0 delay
helpers, dropping to 1200 baud) are **deprecated** — kept below only as
background on *why* a bit-banged UART is hard on this chip.

Two things made the assembly version reliable:

1. **Cycle-exact bit periods.** Each bit is timed by the `delay_8` macro in
   `Library/registers.S` (a counted `dec`/`brne` loop), not by compiler-emitted
   delay code whose length varies with optimization level.
2. **The `ror` instruction.** Receiving shifts each sampled bit (carry) straight
   into the result register with a single `ror` — exactly matching the
   LSB-first serial format, with no `1<<i` mask computed on a chip that has no
   barrel shifter.

The `OSCCAL` calibration story below is still relevant: use `examples/osccal/`
to find your chip's trim value, then set it as `TRIM` in `Library/serial.S`
(default `0x60`), where `init_serial` applies it.

## The current implementation

| Item | Where |
|---|---|
| Assembly primitives | `Library/serial.S` — `init_serial`, `char_write`, `char_read`, `flash_write` |
| Logical register names + `delay_8` macro | `Library/registers.S` |
| C prototypes | `Library/serial_asm.h` |
| C example | `examples/softserial/` (`main.c`) |
| Assembly example | `examples/asm_softserial/` (`main.S`) |
| Calibration helper | `examples/osccal/` |

### Serial constants in *Library/registers.S*:

```asm
; ---------- Serial Communications ----------
; 1. Define the TX/RX pins
; 2. Leave period/half-period alone, unless changing baud rate
; 3. If communications are flaky, use examples/osccal to determine TRIM
; 4. Default: 9600 baud, 8 data bits, 1 stop bit and 0 parity bits
#define TX          PB2   ; transmit pin, output - goes to cable RX
#define RX          PB1   ; receive pin, input pullup - goes to cable TX
#define period       37   ; # of ticks for 1 bit period (9600 baud @ 1.2MHz)
#define half_period  20   ; # of ticks for  a .5 bit period
#define TRIM         0x60 ; OSCCAL trim value, use examples/osccal to determine
#define no_bits     8     ; no of bits, typically 8
```

From C, the whole port is four declarations in `serial_asm.h`:

```c
void    init_serial(void);        // apply TRIM, set pin directions/idle
void    char_write(uint8_t c);    // transmit one byte, 8-N-1
uint8_t char_read(void);          // block for one byte, return it
void    flash_write(uint16_t a);  // print a null-terminated PROGMEM string (Z = a)
```

For how the assembly is made callable from C and how `ASM_LIBS` links the
shared `serial.S` into either a C or an assembly example, see
[`docs/asm_from_c.md`](asm_from_c.md).

---

## Background: why a software UART is hard here

The remaining sections explain the timing problems that drove the move to
assembly. They are still useful for understanding the design, but the code
snippets are illustrative C from the exploration phase, not the shipping
implementation.

There are three core issues:

1. The internal RC oscillator is imprecise (±10 % uncalibrated), whereas an
   Arduino Uno R3 uses a 16 MHz ceramic resonator.
2. You must sample each received bit at the correct instant.
3. Bit timing must be exact and uniform across all 8 bits.

### Why write works but read fails

A working transmit but failing receive is a **classic symptom** of timing
error, because the two directions have very different tolerances.

**Transmit:** *You* generate the signal. The receiving PC/USB-UART has hardware
with oversampling (typically 16×) and resynchronizes on every start bit, so it
tolerates roughly ±5 % timing error.

**Receive:** You must sample at precise moments within each bit period, only
once per bit, at a fixed delay. Timing errors **accumulate** across the 8 data
bits — by bit 7 even a 2–3 % error can land the sample in the wrong bit.

The assembly `char_read` addresses this with **mid-bit sampling**: after the
start-bit falling edge it waits `half_period`, re-confirms the start bit is
still low (noise reject), then a full `period` — i.e. ~1.5 bit periods total —
so the first data bit is sampled in its middle. Each subsequent bit is one
exact `period` later.

### Calibrating the RC oscillator (still used)

The internal oscillator's tolerance directly skews any cycle-counted delay, so
the effective baud rate drifts chip-to-chip. Calibrating `OSCCAL` brings
accuracy to roughly ±2 %, which is well within the ~±5 % a UART needs.

`examples/osccal/` sweeps `OSCCAL` values and emits a test pattern so you can
find the value at which output is clean for your individual chip. Note that
value and set it as `TRIM` in `Library/serial.S`; `init_serial` writes it to
`OSCCAL` at startup:

```asm
    ldi     temp_r18, TRIM
    out     RCCAL, temp_r18      ; RCCAL = _SFR_IO_ADDR(OSCCAL)
```

Tips:
- The factory default `OSCCAL` is typically around `0x6A`–`0x6F`.
- The ATtiny13A has separate calibration ranges for 4.8 MHz and 9.6 MHz.
- Small adjustments (±5) are usually enough.
- If *every* value shows corruption, the baud period itself is off — re-tune
  `period` (each unit ≈ 3 CPU cycles of the `delay_8` loop).


### References

[^1]: [ATtiny Oscillator Calibration — Instructables](https://www.instructables.com/ATtiny-Oscillator-Calibration/)
[^2]: [`<util/delay.h>` — avr-libc](https://www.nongnu.org/avr-libc/user-manual/group__util__delay.html)
[^3]: [ATtiny13A Data Sheet](https://www.farnell.com/datasheets/1714641.pdf)
[^4]: [Serial Data ATtiny13A — Arduino Forum](https://forum.arduino.cc/t/serial-data-attiny13a/1094674)
[^5]: [ATtiny13 internal oscillator accuracy — Arduino Forum](https://forum.arduino.cc/t/attiny13-internal-oscillator-accurate-9-6mhz-but-inaccurate-4-8mhz/674396)
