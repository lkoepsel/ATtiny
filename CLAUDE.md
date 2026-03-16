# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a framework for programming the **ATtiny13A** microcontroller in C (gnu99), using avr-gcc, avr-gdb, and avrdude. The ATtiny13A has 1KB Flash, 64 bytes RAM, and no bootloader — it requires an external ISP programmer (ATMEL-ICE or Microchip SNAP).

## Build System

All examples share a single root `Makefile`. Each example has a minimal 3-line local `Makefile`:

```makefile
DEPTH = ../../
include $(DEPTH)Makefile
```

**Key targets (run from within an example directory):**

```bash
make compile      # Compile only (verify)
make flash        # Compile and upload to device
make complete     # Clean, compile, and upload
make size         # Show Flash/SRAM usage
make clean        # Remove build artifacts
make disasm       # Generate assembly listing
make static       # Run cppcheck static analysis
make stack        # Compile with -fstack-usage, generates main.su
make env          # Print active configuration variables
make verbose      # Flash with detailed avrdude output
```

## env.make (Required, Not in Git)

Each developer must create `env.make` in the repo root before building. Key variables:

```makefile
MCU = attiny13a
F_CPU = 1200000UL          # Default clock; up to 9600000UL available
SERIAL = /dev/ttyACM0      # Adjust per system
PROGRAMMER_TYPE = atmelice_isp   # or snap_isp
PROGRAMMER_ARGS = -F -V -P usb -b 115200
LIBDIR = $(DEPTH)Library
```

See `docs/env_make.md` for full documentation.

## Compilation Flags

- Standard: `-std=gnu99`
- Debug: `-Og -ggdb3` (default); use `-Os` for production
- Warnings: `-Wall -Wundef -Werror`
- Size optimization: `-ffunction-sections -fdata-sections -Wl,--gc-sections`

## Architecture

### Library (`Library/ATtiny.h`)

The core library provides inline assembly macros to avoid compiler optimization pitfalls:

- `SBI(port, bit)` — set bit (maps to AVR `sbi` instruction)
- `CBI(port, bit)` — clear bit (maps to AVR `cbi` instruction)
- `TIMER_DELAY(ticks)` — busy-wait using TCNT0

The `"I"` constraint enforces compile-time constants (0–63) required by the `sbi`/`cbi` AVR instructions.

### Example Structure

Each example in `examples/` is self-contained: a `main.c`, a local `Makefile`, and any needed headers copied locally (e.g., `ATtiny.h`, `soft_serial.h`, `sysclock.h`). There is no shared linking — headers are duplicated by design to keep examples independent.

### Key Headers (found in relevant examples)

- `ATtiny.h` — SBI/CBI/TIMER_DELAY macros
- `sysclock.h` — `init_sysclock_1k()`, `ticks()` for 1kHz system clock using Timer0 overflow interrupt
- `soft_serial.h` — Software UART at 1200 baud: `init_soft_serial()`, `soft_char_write()`, `soft_char_read()`, `soft_readLine()`, `soft_int16_write()`, `soft_pgmtext_write()`

### Hardware Constraints

- **Flash:** 1024 bytes — keep code small; use `-Os` for production builds
- **RAM:** 64 bytes — avoid dynamic allocation, minimize stack usage
- **Pins:** PB0–PB5; PB5 is RESET (avoid as GPIO unless fuses are changed)
- **ADC:** 4 channels on PB2–PB5, 10-bit, ~260µs conversion
- **PWM:** OC0A (PB0) and OC0B (PB1) via Timer0
- **Clock:** Default 1.2MHz (factory fuse). Higher speeds require fuse changes and env.make update.

## Debugging

Uses Bloom + avr-gdb over debugWire (single-wire protocol on RESET pin).

- Fuse must be set to enable debugWire: changes `PROGRAMMER_TYPE` to `atmelice_dw`
- `make verbose` shows full avrdude output for upload troubleshooting
- See `docs/bloomandgdb.md` for full setup

## Static Analysis

```bash
make static   # Runs cppcheck using test.cppcheck config and suppressions.txt
```

## Stack Analysis

`make stack` compiles with `-fstack-usage` and generates `main.su` in the example directory:

```
main.c:line:col:function_name  frame_bytes  static|dynamic|dynamic,bounded
```

Worst-case stack depth = sum of frame sizes along the deepest call chain, plus ~4–6 bytes per
active ISR. With only 64 bytes of RAM, always verify stack headroom after adding new functions or
ISRs. See `docs/analysis.md` for a worked example using `examples/reaction`.

To add the target, append to the root `Makefile` after the `compile` target:

```makefile
stack: CFLAGS += -fstack-usage
stack: $(TARGET).hex
```
