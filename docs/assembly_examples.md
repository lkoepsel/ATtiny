# AVR Assembly Examples

Hand-written AVR assembly for the **ATtiny13A**. Each example is a self-contained folder
with a `main.S` source and a 2-line `Makefile`.

For framework-wide setup (toolchain install, `env.make`, programmer wiring) see the repo-root
`CLAUDE.md` and `docs/`. For a line-by-line toolchain walkthrough see `docs/assembly.md`.

## How assembly is built

Sources are named **`main.S`** — uppercase `.S`. That extension tells `avr-gcc` to run the C
preprocessor before assembling, which is why these files can use `#include` and `#define`.

Every `main.S` begins with:

```asm
#include <avr/io.h>      ; avr-libc register & bit names (DDRB, PB0, RAMEND, ...)
#include "registers.S"   ; project logical names (LED, IO_DDR, STATUS, ...)
```

`registers.S` and `serial.S` are shared from `Library/` and located via the `-I` flag in
`Makefile.asm`, so an example only needs its own `main.S` and `Makefile`.

One rule when reading or writing these files: the `in`, `out`, `sbi`, and `cbi` instructions
need their register operand wrapped in `_SFR_IO_ADDR()`, because `avr/io.h` defines register
names as data-space addresses. `registers.S` already does this for its logical names — so
prefer `sbi IO_DDR, LED` over `sbi DDRB, PB0`.

Build chain (`Makefile.asm`): `avr-gcc` assembles `main.S` → `main.o`, links with
`-nostartfiles -nostdlib` → `main.elf`, then `avr-objcopy` produces `main.hex`.

## Common targets

Run from inside any example folder:

```bash
make compile   # assemble + build main.hex
make flash     # build and upload to the device
make complete  # clean, rebuild, show size
make size      # Flash / SRAM usage
make disasm    # generate main.lst listing
make clean     # remove build artifacts
make help      # full target list
```

## Creating a new program

Start from `skeleton/` — a minimal program with the interrupt vector table and reset handler
already in place:

```bash
cp -r asm_examples/skeleton asm_examples/myprogram
cd asm_examples/myprogram
```

Then:

1. Edit `main.S` — add setup code under `main_setup` and loop logic under `main_loop`.
2. Need a register or pin name that isn't in `Library/registers.S`? Add it there, or drop a
   local `registers.S` in the example folder to override the shared one.
3. `make compile` to assemble, `make flash` to upload.

The `Makefile` is the standard 2-liner and needs no editing:

```makefile
DEPTH = ../../
include $(DEPTH)Makefile.asm
```

## Examples

| Folder | What it does |
|---|---|
| `skeleton` | Minimal template — vector table, reset handler (stack-pointer init, zero register, cleared status), empty `main_loop`. Copy this to start a new program. |
| `blink` | Blinks an LED on PB0 at 500 Hz using a software (cycle-counted) busy-wait delay. See `blink/blink.md` for a line-by-line analysis. |
| `blink_nodelay` | Blinks the LED with a **non-blocking** Timer0 poll instead of a busy-wait — Timer0 prescaler 1024, ~100 ms signal. |
| `blink_word` | Blinks the LED at 500 Hz using a 16-bit (word) counter; trimmed to minimal flash size by omitting unused interrupt vectors. See `blink_word/blink_word.md`. |
| `blink_pwm` | Drives the LED with a hardware **PWM** signal from Timer0 (~4.51 kHz, no prescaler). |
| `serial` | Software-UART demo; pulls in the shared `serial.S` routines from `Library/`. |
| `sysclock` | Non-blocking **1 kHz system clock** via a Timer0 CTC interrupt; uses serial to report timing. See `sysclock/sysclock.md`. |

Per-example `.md` files (`blink.md`, `blink_word.md`, `sysclock.md`) contain deeper
walkthroughs where present.
