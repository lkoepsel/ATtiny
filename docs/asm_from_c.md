# Calling AVR Assembly from C — Using `serial.S` from a C Program

This guide explains how the hand-written AVR assembly in `Library/serial.S` is
made callable from C, so that the cycle-deterministic `char_write` and
`char_read` (plus `init_serial` and `flash_write`) can be used from any C
program. The same shared `serial.S` is linked by both the C example
(`examples/softserial/`) and the assembly example (`examples/asm_softserial/`).

By the end you will understand:

- why the bit-banged UART has to be assembly,
- how `serial.S` satisfies the AVR-GCC calling convention,
- how `Library/serial_asm.h` declares the routines for C, and
- how the `ASM_LIBS` mechanism in the unified root `Makefile` links one shared
  assembly object into either kind of example.

## Why this matters

A pure-C software UART is unreliable because a bit-banged UART is *hard real
time*: every bit period must be a uniform, exact number of CPU cycles. A C
per-bit period is `_delay_us(...)` plus a *variable* amount of
compiler-generated surrounding code (loop housekeeping, `1<<i` evaluated
without a barrel shifter, optimization-level-dependent overhead) — so the bit
period drifts and reception breaks.

`Library/serial.S` runs a constant, hand-counted number of cycles per bit (via
the `delay_8` macro). Exposing it to C gives the timing accuracy of
hand-tuned assembly with the convenience of writing the application logic
(string formatting, line buffering, integer-to-ASCII conversion) in plain C.
All the timing-critical work stays in assembly; the C code only sequences bytes
through the primitives, and any compiler-generated overhead lands in the *gap*
between characters (inside the stop bit + idle), where it cannot break the
protocol.

---

## Background — the avr-gcc calling convention (ABI)

To make an assembly function safely callable from C, the assembly has to follow
the AVR-GCC ABI: a fixed contract about which registers carry arguments, which
carry return values, and which must be preserved across a call.

### Register roles

| Range | Role | Notes |
|---|---|---|
| `r0` | Scratch | Not preserved. Used internally by some instructions (e.g. `mul`). |
| `r1` | **Zero register** | Must equal 0 on function entry and exit. If you clobber it, restore with `eor r1, r1`. |
| `r2`–`r17` | **Call-saved** | If the callee uses these, it must `push` them on entry and `pop` them before `ret`. |
| `r18`–`r27` | **Call-clobbered** | The callee may use them freely; the caller does not expect them preserved. |
| `r28`, `r29` | **Call-saved (Y register)** | The compiler uses Y as a frame pointer. Preserve if touched. |
| `r30`, `r31` | **Call-clobbered (Z register)** | Free for use by the callee. |

### Argument and return registers

| | Where it lives |
|---|---|
| 1st argument | `r24` (8-bit) or `r25:r24` (16-bit) |
| 2nd argument | `r22` or `r23:r22` |
| 3rd argument | `r20` or `r21:r20` |
| 4th argument | `r18` or `r19:r18` |
| 8-bit return value | `r24` |
| 16-bit return value | `r25:r24` |

> **Pattern:** byte arguments occupy even-numbered call-clobbered registers,
> right-to-left from r24 down. A 16-bit pointer argument (used by
> `flash_write`) arrives in `r25:r24`, but `flash_write` instead expects its
> Z-pointer already loaded in `r31:r30` — see below.

### The contract, in one sentence

> The caller may put anything in r0, r18–r27, r30, r31; the callee may use them
> freely. Everything else (r1, r2–r17, r28, r29) must look the same before and
> after the call.

---

## How `Library/serial.S` meets the ABI

`serial.S` uses **logical register names** defined in `Library/registers.S`
rather than raw `rNN` numbers. The relevant aliases are:

```asm
#define temp_r18    r18      ; scratch
#define bit_ctr     r20      ; bit / loop counter
#define char_reg    r24      ; the character (ABI arg/return register)
#define flash_lo    r30      ; Z low  (PROGMEM pointer)
#define flash_hi    r31      ; Z high
```

`registers.S` also defines the cycle-counting delay used for each bit period:

```asm
.macro  delay_8  ticks
    ldi     r19, \ticks
9:  dec     r19
    brne    9b
.endm
```

Every register the routines touch — `r18`, `r19` (inside `delay_8`),
`r20`, `r24`, and `r30`/`r31` — is **call-clobbered**, so the routines need no
`push`/`pop` of call-saved registers, and `r1` is never disturbed. That is what
makes them ABI-clean and directly callable from C.

### The four exported routines

Each is marked `.global` so the linker can find it from a C object file:

| Routine | C prototype | ABI mapping |
|---|---|---|
| `init_serial` | `void init_serial(void)` | No args/return. Applies the OSCCAL `TRIM`, sets TX/RX pin directions and idle state. Must be called first. |
| `char_write` | `void char_write(uint8_t c)` | Character arrives in `char_reg` (r24); transmits 8N1. |
| `char_read` | `uint8_t char_read(void)` | Blocks for one byte, returns it in `char_reg` (r24). |
| `flash_write` | `void flash_write(uint16_t addr)` | Z-pointer (`r31:r30`) addresses a null-terminated PROGMEM string; each byte is sent via `char_write`. |

`delay_8` is a macro (inlined at each call site), so there is no separate
private helper to keep out of the global namespace.

### Design decision: no framing-error status

An earlier version of `char_read` returned a framing-error flag in the SREG
**T** bit. C has no direct access to single SREG bits, so the current
`char_read` **drops the status** and simply returns the received byte — mirroring
the behaviour of the old C `soft_char_read`. If you ever need error reporting
back, the idiomatic approach is to widen the return to `uint16_t` (low byte =
char, high byte = status) and set `r25` before `ret`.

---

## The C header — `Library/serial_asm.h`

C code includes this header to declare the assembly routines. It already lives
in `Library/` and is found via the `-I$(LIBDIR)` include path:

```c
// serial_asm.h
// C declarations for the assembly routines in serial.S
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialise TX/RX pin directions and idle state.
// Must be called before char_write or char_read.
void init_serial(void);

// Transmit one byte at 9600-8-N-1.
// The character is passed in r24 per the AVR-GCC ABI.
void char_write(uint8_t c);

// Block until one byte is received; return it.
// The result is returned in r24 per the AVR-GCC ABI.
uint8_t char_read(void);

// Write program memory text to console.
// The address is passed in r31/r30.
void flash_write(uint16_t addr);

#ifdef __cplusplus
}
#endif
```

Notes:

- **No name mangling.** AVR-GCC does not mangle C function names, so the asm
  label `char_write` matches the C declaration `char_write` directly. The
  `extern "C"` wrapper only matters if you compile as C++.
- **`uint8_t` vs `char`.** `uint8_t` makes the byte-orientation explicit and
  avoids accidental sign extension when bytes widen to `int` in expressions.

---

## How the build links the shared assembly — the `ASM_LIBS` mechanism

Both root makefiles share one idea: an example's **local Makefile** sets the
`ASM_LIBS` variable to any shared `.S` files it wants linked in, and the root
makefile appends those to the object list. The shared `serial.S` is **not
copied** into the example — it is referenced in place from `Library/`, so there
is a single shared assembly source used identically by C and asm callers.

### In the root C `Makefile`

```makefile
SOURCES     = $(wildcard *.c)
ASM_SOURCES = $(wildcard *.S) $(ASM_LIBS)
CPPFLAGS    = -DF_CPU=$(F_CPU) -DUSB_BAUD=$(USB_BAUD) -DSOFT_BAUD=$(SOFT_BAUD) -I. -I$(LIBDIR)

OBJECTS = $(SOURCES:.c=.o) $(ASM_SOURCES:.S=.o)
```

with the pattern rule that assembles `.S` through the C preprocessor:

```makefile
## Assemble .S files (uppercase S runs the C preprocessor first)
%.o: %.S Makefile
	$(CC) $(CPPFLAGS) $(TARGET_ARCH) -g -Wa,--gdwarf-2 -MMD -MP -c -o $@ $<
```

Why these flags:

- **`$(CPPFLAGS)`** — supplies `-DF_CPU=...` and the include paths `-I.` and
  `-I$(LIBDIR)`. The preprocessor needs them because `serial.S` does
  `#include "registers.S"`, and `registers.S` uses `_SFR_IO_ADDR(...)` (from
  `<avr/io.h>`) to turn register names into numeric I/O addresses.
- **`$(TARGET_ARCH)`** — `-mmcu=$(MCU)`, selecting the correct device
  definitions.
- **`-g`** — debug info, parallel to what C objects get.
- **No `$(CFLAGS)`** — those (`-std=gnu99`, `-Wundef`, `-fpack-struct`, …) are
  C-only and either meaningless or harmful for assembly.

Because the `.S` is referenced via its `$(DEPTH)Library/...` path, the object is
built next to it as `Library/serial.o`; the `all_clean` target removes
`$(LIBDIR)/*.o` so it is not left stale.

### The freestanding link for asm-only examples

The same root `Makefile` builds the assembly example. Since `examples/asm_softserial/`
has no `.c` sources, the Makefile auto-selects the *freestanding* link model:

```makefile
ifeq ($(strip $(SOURCES)),)
  FREESTANDING ?= 1
else
  FREESTANDING ?= 0
endif

ifeq ($(FREESTANDING),1)
  LDFLAGS = -nostartfiles -nostdlib
endif
```

So the asm example links `Library/serial.S` as a **separate object** (via the same
`ASM_LIBS` / `%.o: %.S` mechanism) rather than `#include`-ing it — exactly the
shared-object arrangement the C side uses. The `-nostartfiles -nostdlib` link keeps
the hand-written vector table and reset code authoritative (no C runtime).

> **Indentation note:** the leading whitespace on a make recipe line must be a
> real tab, not spaces. If a copied rule silently doesn't fire, that is almost
> always the cause.

---

## The two examples that use `serial.S`

Each example is just a `main.*` plus a two-or-three-line local Makefile that
points `ASM_LIBS` at the shared `Library/serial.S`.

### C example — `examples/softserial/`

**`Makefile`**
```makefile
DEPTH = ../../
ASM_LIBS = $(DEPTH)Library/serial.S
include $(DEPTH)Makefile
```

**`main.c`** (echo, with a PROGMEM prompt written in plain C):
```c
#include <avr/pgmspace.h>
#include "serial_asm.h"

#define CR 13
#define LF 10

const char prompt[]  PROGMEM = "13A";
const char waiting[] PROGMEM = "W:";

// Write a PROGMEM-resident, null-terminated string to the serial port.
static void pgmtext_write(const char *p)
{
    for (uint8_t c; (c = pgm_read_byte(p)); p++)
        char_write(c);
}

int main(void)
{
    init_serial();

    char_write(CR);
    char_write(LF);
    pgmtext_write(prompt);
    char_write(CR);
    char_write(LF);
    pgmtext_write(waiting);

    // Echo each received character back over the serial port.
    for (;;) {
        char_write(char_read());
    }
}
```

Note the higher-level helper (`pgmtext_write`) is ordinary C over the two
primitives. (The assembly `flash_write` does the same job in asm and is also
available from C if you prefer it.)

### Assembly example — `examples/asm_softserial/`

**`Makefile`**
```makefile
DEPTH = ../../
ASM_LIBS = $(DEPTH)Library/serial.S
include $(DEPTH)Makefile
```

`main.S` provides the interrupt vector table and reset handler, then calls the
same exported routines. Because both `char_read` and `char_write` use
`char_reg` (r24), the received byte flows straight from one to the other with
no `mov`:

```asm
main_loop:
    rcall   char_read
    rcall   char_write
    rjmp    main_loop
```

---

## Build, flash, verify

For either example:

```bash
cd examples/softserial        # or examples/asm_softserial
make complete
make flash
```

Inspect the build output and confirm:

- A line like `avr-gcc ... -c -o ../../Library/serial.o ../../Library/serial.S`
  appears (proves `ASM_LIBS` and the `.S` pattern rule fired).
- Final flash size is small — the asm primitives are tiny.

Then connect a USB-to-serial adapter and open a terminal at **9600-8-N-1**
(e.g. `tio -b 9600 /dev/ttyUSB0` or `screen /dev/ttyUSB0 9600`):

- `init_serial()` runs cleanly (no startup garbage; the C example prints its
  `13A` / `W:` prompt).
- Typed characters echo back exactly, with no bit drops at sustained typing
  speed.

If reception is unreliable, see the timing section below.

---

## Tuning timing if needed

The bit period is set by two cycle counts at the top of `Library/serial.S`,
together with an OSCCAL trim value applied in `init_serial`:

```asm
#define period       37      ; # of ticks for 1 bit period (9600 baud @ 1.2 MHz)
#define half_period  20      ; # of ticks for a 0.5 bit period
#define TRIM         0x60    ; OSCCAL trim value, use examples/osccal to determine
```

`period = 37` was tuned for an ATtiny13A at the 1.2 MHz internal RC clock. The
internal oscillator has ±10 % tolerance uncalibrated, so the effective baud
shifts chip-to-chip. Two avenues:

1. **Calibrate the chip** — `init_serial` writes `TRIM` to `OSCCAL` at startup
   to nudge the oscillator toward true 1.2 MHz. Use `examples/osccal/` to find
   the right `TRIM` for your individual chip, then update the `TRIM` define.
2. **Re-tune `period`** — measure the bit period on a logic analyser and scale
   `period` proportionally. Each unit of `period` is the cost of one
   `delay_8` loop iteration (`dec; brne` taken ≈ 3 CPU cycles).

Pick one avenue and stick with it for repeatability.

---

## Reference — where everything lives

| File | Role |
|---|---|
| `Library/serial.S` | The ABI-clean primitives: `init_serial`, `char_write`, `char_read`, `flash_write`. Uses logical register names + `delay_8` from `registers.S`. |
| `Library/registers.S` | Logical register aliases (`char_reg`, `temp_r18`, `bit_ctr`, `flash_lo/hi`) and the `delay_8` macro. Include-guarded. |
| `Library/serial_asm.h` | C prototypes for the four exported routines. |
| `Makefile` (unified root) | `ASM_SOURCES = $(wildcard *.S) $(ASM_LIBS)`; `%.o: %.S` pattern rule; auto-detects the link model — runtime link when `.c` sources exist, freestanding (`-nostartfiles -nostdlib`) when only `.S`. |
| `examples/softserial/` | C example: `main.c` + local Makefile setting `ASM_LIBS`. |
| `examples/asm_softserial/` | Assembly example: `main.S` (vectors + reset) + local Makefile setting `ASM_LIBS`. |

---

## Where to go from here

With `char_write` and `char_read` callable from C, every higher-level helper is
ordinary C over those two primitives — for example:

```c
void soft_string_write(const char *s)
{
    while (*s) char_write((uint8_t)*s++);
}

void soft_uint16_write(uint16_t n)
{
    char buf[6];
    itoa(n, buf, 10);
    soft_string_write(buf);
}
```

Add such helpers in the example's `main.c` (or a small companion `.c`); the
timing-critical work stays in `Library/serial.S`, shared unchanged by both the C
and assembly examples.
