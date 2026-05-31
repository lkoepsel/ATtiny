# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a framework for programming the **ATtiny13A** microcontroller in C (gnu99) and AVR assembly, using avr-gcc, avr-gdb, and avrdude. The ATtiny13A has 1KB Flash, 64 bytes RAM, and no bootloader — it requires an external ISP programmer (ATMEL-ICE or Microchip SNAP).

## Build System

C and assembly examples share the same `make` targets but use separate root Makefiles:

| | C examples (`examples/`) | Assembly examples (`asm_examples/`) |
|---|---|---|
| Root Makefile | `Makefile` | `Makefile.asm` |
| Source file | `main.c` | `main.S` |
| Local Makefile | `DEPTH = ../../`<br>`include $(DEPTH)Makefile` | `DEPTH = ../../`<br>`include $(DEPTH)Makefile.asm` |

**Key targets (run from within any example directory):**

```bash
make compile          # Compile/assemble only
make flash            # Build and upload to device
make complete         # Full rebuild: clean, build, show size (no upload)
make size             # Show Flash/SRAM usage
make clean            # Remove build artifacts
make disasm           # Generate assembly listing
make env              # Print active configuration variables
make verbose          # Flash with detailed avrdude output
make help             # Print available targets
make show_fuses       # Read current fuse values from device
make set_fast_fuse    # Set LFUSE to 0xE2 (disable CLKDIV8, run at 9.6MHz)
make avrdude_terminal # Open interactive avrdude terminal
```

**C-only targets:**

```bash
make static           # Run cppcheck static analysis
make stack            # Compile with -fstack-usage, generates main.su
make flash_eeprom     # Flash EEPROM from main.eeprom
```

## env.make (Required, Not in Git)

Each developer must create `env.make` in the repo root before building. Key variables:

```makefile
MCU = attiny13a
F_CPU = 1200000UL          # Default clock; up to 9600000UL available
SERIAL = /dev/ttyACM0      # Adjust per system
PROGRAMMER_TYPE = atmelice_isp   # or snap_isp
PROGRAMMER_ARGS = -F -V -P usb -b 115200
USB_BAUD = 250000UL
SOFT_BAUD = 28800UL
```

`env.make` is for values that genuinely vary by machine. Repo-structure paths
like `LIBDIR = $(DEPTH)Library` live in the tracked root `Makefile` /
`Makefile.asm`, so every developer sees the same layout automatically.

See `docs/env_make.md` for full documentation.

## Compilation Flags

These flags apply to **C examples**. Assembly examples use a separate pipeline — see below.

- Standard: `-std=gnu99`
- Debug: `-Og -ggdb3` (default); use `-Os` for production
- Warnings: `-Wall -Wundef -Werror`
- Size optimization: `-ffunction-sections -fdata-sections -Wl,--gc-sections`

## Assembly Examples

Assembly examples in `asm_examples/` are written as **`main.S`** (uppercase `.S`) and built
with `avr-gcc`, not `avr-as`. The uppercase extension makes `avr-gcc` run the C preprocessor
before assembling, so `#include`, `#define`, and `-D` definitions all work.

- **Includes:** every `main.S` starts with `#include <avr/io.h>` (avr-libc register/bit
  names) and `#include "registers.S"` (project-defined *logical* names).
- **Shared headers:** `registers.S` and `serial.S` live in `Library/` and are found via
  the `-I$(DEPTH)Library` flag in `Makefile.asm`. An example may also keep a local
  `registers.S` to override them — unlike C examples, asm examples do not duplicate these.
- **`_SFR_IO_ADDR()`:** inside a `.S` file, `avr/io.h` defines register names as *data-space*
  addresses. The `in`/`out`/`sbi`/`cbi` instructions need *I/O-space* addresses, so operands
  must be wrapped in `_SFR_IO_ADDR()`. `registers.S` applies this wrapping for the names it
  defines (e.g. `IO_DDR`, `STATUS`).
- **Build:** `Makefile.asm` assembles with `avr-gcc -mmcu=... -MMD -MP` (automatic header
  dependency tracking) and links with `avr-gcc -nostartfiles -nostdlib`, keeping the
  hand-written vector table and reset code authoritative (no C runtime is linked).
- **New example:** copy `asm_examples/skeleton/` — a minimal template with the vector table
  and reset handler already in place.

See `docs/assembly.md` for the full toolchain walkthrough and `asm_examples/README.md` for a
per-example index.

## Architecture

### Library (`Library/ATtiny.h`)

The core library provides inline assembly macros to avoid compiler optimization pitfalls:

- `SBI(port, bit)` — set bit (maps to AVR `sbi` instruction)
- `CBI(port, bit)` — clear bit (maps to AVR `cbi` instruction)

The `"I"` constraint enforces compile-time constants (0–63) required by the `sbi`/`cbi` AVR instructions.

**Pin toggle shortcut:** Writing to `PINB` toggles the corresponding output bit — `sbi PINB, n` toggles pin n in a single instruction. Used extensively throughout examples for efficient LED toggling.

### Example Structure

Each example in `examples/` is self-contained: a `main.c`, a local `Makefile`, and any needed files copied locally (e.g., `ATtiny.h`, `soft_serial.h`, `soft_serial.c`, `sysclock.h`, `sysclock.c`). There is no shared linking — files are duplicated by design to keep examples independent.

### Key Modules (copy both `.h` and `.c` into example directory when needed)

- `ATtiny.h` — SBI/CBI macros (header only)
- `sysclock.h` / `sysclock.c` — `init_sysclock_1k()`, `ticks()` for 1kHz system clock using Timer0 CTC interrupt; **both files must be copied** to use in an example
- `soft_serial.h` / `soft_serial.c` — Software UART at 1200 baud: `init_soft_serial()`, `soft_char_write()`, `soft_char_read()`, `soft_readLine()`, `soft_int16_write()`, `soft_pgmtext_write()`; **both files must be copied**

### Hardware Constraints

- **Flash:** 1024 bytes — keep code small; use `-Os` for production builds
- **RAM:** 64 bytes — avoid dynamic allocation, minimize stack usage
- **Pins:** PB0–PB5; PB5 is RESET (avoid as GPIO unless fuses are changed)
- **ADC:** 4 channels on PB2–PB5, 10-bit, ~260µs conversion
- **PWM:** OC0A (PB0) and OC0B (PB1) via Timer0
- **Clock:** Default 1.2MHz (factory fuse: 9.6MHz oscillator with CLKDIV8 enabled). Higher speeds require fuse changes and env.make update.

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
