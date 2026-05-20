# Calling AVR Assembly from C — Using `softserial.S` from a C Program

This guide walks through the changes needed to call hand-written AVR assembly
routines from C, using the existing `Library/softserial.S` as the working
example. By the end, the assembly `char_write` and `char_read` (with their
cycle-deterministic timing) will be callable from any C program, and the
higher-level helpers — `soft_string_write`, `soft_int16_write`,
`soft_readLine`, `soft_pgmtext_write` — can be rewritten in C against those
primitives.

## Why this matters

The C version of software serial (`examples/softserial/soft_serial.c`) is
unreliable because a bit-banged UART is *hard real-time*: every bit period
must be a uniform, exact number of CPU cycles. The C version's per-bit period
is `_delay_us(BIT_DURATION)` plus a *variable* amount of compiler-generated
surrounding code (loop housekeeping, `1<<i` evaluated without a barrel
shifter, optimization-level-dependent overhead) — so the bit period drifts
and reception breaks.

The assembly version in `Library/softserial.S` runs in a constant,
hand-counted number of cycles per bit. By exposing it to C, we get the timing
accuracy of hand-tuned assembly with the convenience of writing the
application logic (string formatting, line buffering, integer-to-ASCII
conversion) in plain C.

---

## Background — the avr-gcc calling convention (ABI)

To make an assembly function safely callable from C, the assembly has to
follow the AVR-GCC ABI. This is a fixed contract about which registers carry
arguments, which carry return values, and which must be preserved across a
call. Five rules cover almost everything.

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
> right-to-left from r24 down. Additional argument slots exist (r9:r8 etc.)
> but ATtiny13A programs almost never push that many.

### The contract, in one sentence

> The caller may put anything in r0, r18–r27, r30, r31; the callee may use
> them freely. Everything else (r1, r2–r17, r28, r29) must look the same
> before and after the call.

Keep that contract in mind as you audit the assembly.

---

## Step 1 — Audit `Library/softserial.S` against the ABI

Open `Library/softserial.S` and check each routine against the table above.

**`char_write`**
- Header comment says "char in r17". **r17 is call-saved** — passing a C
  argument there is non-standard, and clobbering it would violate the
  contract. → The char must arrive in **r24**.
- Uses `r17`, `r18`, `r19`, `r20`. r17 has to go; r18/r19/r20 are fine
  (call-clobbered).

**`char_read`**
- Returns the char in **r17** and a framing-error flag in **T**. The ABI
  return register is **r24**. The T flag has no direct C equivalent.
- Internally uses `r17`, `r18`, `r19`, `r20`. Same issue with r17.

**`init_serial`**
- No arguments, no return. Touches only I/O registers. Already ABI-clean.

**`timer_delay`**
- Internal helper called only by `char_write` and `char_read`. Stays private
  to the file (no `.global`). Takes its count in `r19`; r19 is call-clobbered
  anyway, so no ABI concern.

**r1**
- None of the current routines touch r1. ✓
- (If you ever add a `mul` instruction or call a C function from the asm,
  remember `mul` writes to r1:r0 and you must zero r1 afterward.)

### Decision: what to do with the T-flag status

`char_read` currently sets the T flag to indicate a framing error. C has no
direct access to single SREG bits, so we need a different mechanism. Three
options, in increasing complexity:

| Option | Pros | Cons |
|---|---|---|
| **Drop the status** | Simplest; matches the existing C `soft_char_read`, which ignores errors | Loses error detection |
| Pack into a `uint16_t` return (low byte = char, high byte = status) | One return value, no extra arg | Slightly more code on both sides |
| Status-pointer arg: `char_read(uint8_t *err)` | Idiomatic C | Extra arg, extra register, extra memory write |

**This guide uses the first option (drop the status).** It mirrors the
existing C behaviour and keeps the asm change minimal. The "keep error
reporting" alternative appears in a callout in Step 2c.

---

## Step 2 — Rework `Library/softserial.S` to the ABI

Three concrete edits.

### 2a. Make `char_write` take its argument in r24

In the current code:

```asm
char_write:
    ; Start bit
    cbi     IO_PORT, TX
    ldi     r19,period
    rcall   timer_delay

    ;  8 data bits and preserve char
    ldi     r20, 8
    mov     r18, r17          ; ← reads from r17
    ...
```

Change exactly one line:

```diff
-    mov     r18, r17
+    mov     r18, r24
```

That's the entire ABI fix for `char_write`. `r24` is call-clobbered, so we're
free to copy it into `r18` and `ror` through `r18` without preserving the
original.

Also update the comment at the top of the routine so the convention is
documented:

```diff
-; write a char in r17 to serial port, r17 is preserved
+; write a char (passed in r24, per AVR-GCC ABI) to the serial port
```

### 2b. Make `char_read` return its result in r24

The read loop currently rotates received bits into r17:

```asm
read_bit:
    in      r18, IO_PIN
    clc
    sbrc    r18, RX
    sec
    ror     r17                 ; ← accumulates into r17
    ...
```

Change to `r24`:

```diff
-    ror     r17
+    ror     r24
```

Update the comment:

```diff
-; char_read - receive one char into r17 (8N1, LSB first)
-;   Returns status in T flag:  T=0 frame OK,  T=1 framing error.
+; char_read - receive one char into r24 (8N1, LSB first), per AVR-GCC ABI
```

### 2c. Drop the T-flag status

Delete the stop-bit check block at the end of `char_read`, keeping only the
`ret`:

```diff
-;   Stop bit check - line must be HIGH (mark); loop fall-through is ~mid stop bit.
-;   Status returned in T flag:  T=0 frame OK,  T=1 framing error.
-    clt                         ; assume frame OK
-    in      r18, IO_PIN
-    sbrs    r18, RX             ; skip 'set' when stop bit is HIGH (valid)
-    set                         ; stop bit LOW -> framing error
     ret
```

> **Alternative — keep error reporting:**
> If you want to preserve the framing-error check, return `uint16_t` instead:
> place the status (0 or 1) into `r25` before `ret`, and declare the C
> function as `uint16_t char_read(void)`. The low byte (r24) holds the
> character; the high byte (r25) is non-zero on framing error.
>
> ```asm
>     ldi     r25, 0              ; assume frame OK
>     in      r18, IO_PIN
>     sbrs    r18, RX
>     ldi     r25, 1              ; stop bit LOW -> framing error
>     ret
> ```

---

## Step 3 — Export the symbols with `.global`

The linker only finds symbols marked `.global`. Add three lines, one above
each routine's label:

```diff
+.global init_serial
 init_serial:
     ...

+.global char_write
 char_write:
     ...

+.global char_read
 char_read:
     ...
```

`timer_delay` stays private — no `.global` for it. The `.global` directives
are zero-cost (assembler-only) and have no effect on the generated machine
code.

---

## Step 4 — Update the existing asm caller

`asm_examples/softserial/main.S` calls these routines directly. The echo loop
is:

```asm
main_loop:
    rcall   char_read
    brts    main_loop          ; framing error -> discard, wait for next char
    rcall   char_write
    rjmp    main_loop
```

Because the data path is now `char_read` → r24 → `char_write` (both routines
use r24 the same way), **no `mov` is needed** — the character flows through
r24 automatically. The only line that must go is the `brts` framing-error
check, since we dropped the T-flag status:

```diff
 main_loop:
     rcall   char_read
-    brts    main_loop          ; framing error -> discard, wait for next char
     rcall   char_write
     rjmp    main_loop
```

(If you chose the "keep error reporting" alternative in Step 2c, replace
this with a test on `r25` instead, e.g. `tst r25` / `brne main_loop`.)

Also tidy the register-usage notes near the top of the file — r17 is no
longer the char register:

```diff
 ; ---------- Registers and Values ----------------
 ; r16                           ; temp register
-; r17                           ; char register
+; r24                           ; char register (per AVR-GCC ABI)
 ; r18                           ; temp register
 ; r19                           ; timer delay register
```

---

## Step 5 — Verify the asm example still works

Before touching anything on the C side, prove the asm side still works:

```bash
cd asm_examples/softserial
make complete
make flash
```

Connect a USB-to-serial adapter and run a terminal at 9600-8-N-1 (e.g.
`screen /dev/ttyUSB0 9600` or `tio -b 9600 /dev/ttyUSB0`). Type characters —
they should echo back exactly as before. If echo is broken, the ABI changes
regressed timing or register usage; revisit Steps 2a–2c before moving on.

This is the critical checkpoint. **Do not proceed to the C side until the asm
example still works end-to-end.**

---

## Step 6 — Declare the asm routines for C

Create a small header that C code can `#include`.

**`softserial_asm.h`**
```c
// softserial_asm.h
// C declarations for the assembly routines in softserial.S
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

#ifdef __cplusplus
}
#endif
```

Notes:
- **No name mangling.** AVR-GCC does not mangle C function names, so the asm
  label `char_write` matches the C declaration `char_write` directly. The
  `extern "C"` wrapper only matters if you compile the code as C++.
- **`#pragma once` vs include guards.** Either is fine; `#pragma once` is one
  line shorter and works in every C/C++ compiler avr-gcc ships with.
- **`uint8_t` vs `char`.** A signed `char` would also fit, but `uint8_t`
  makes the byte-orientation explicit and avoids accidental sign extension
  when bytes are widened to `int` in expressions.

---

## Step 7 — Teach the root C `Makefile` about `.S` files

The C build currently picks up `*.c` only. Three small edits add `.S`
support without disturbing existing C behaviour.

### 7a. Add an `ASM_SOURCES` variable in both branches

Find the `ifeq ($(LIBRARY),no_lib)` block near the top of `Makefile`:

```diff
 ifeq ($(LIBRARY),no_lib)
     SOURCES=$(wildcard *.c )
+    ASM_SOURCES=$(wildcard *.S )
     CPPFLAGS = -DF_CPU=$(F_CPU) -DUSB_BAUD=$(USB_BAUD) -DSOFT_BAUD=$(SOFT_BAUD)

 else
     SOURCES=$(wildcard *.c $(LIBDIR)/*.c)
+    ASM_SOURCES=$(wildcard *.S $(LIBDIR)/*.S)
     CPPFLAGS = -DF_CPU=$(F_CPU) -DUSB_BAUD=$(USB_BAUD) -DSOFT_BAUD=$(SOFT_BAUD)   -I. \
 	-I$(LIBDIR)
 endif
```

The `no_lib` branch picks up only `.S` files in the current example folder
(consistent with how it picks up `.c` files); the other branch also picks up
`.S` files from `$(LIBDIR)`.

### 7b. Add the asm objects to `OBJECTS`

A few lines below:

```diff
-OBJECTS=$(SOURCES:.c=.o)
+OBJECTS=$(SOURCES:.c=.o) $(ASM_SOURCES:.S=.o)
 HEADERS=$(SOURCES:.c=.h)
```

The link rule (`$(TARGET).elf: $(OBJECTS)`) already links everything in
`OBJECTS`, so nothing else needs touching there.

### 7c. Add a pattern rule for `.S → .o`

Below the existing `%.o: %.c` rule:

```makefile
## Assemble .S files (uppercase S runs the C preprocessor first)
%.o: %.S Makefile
	$(CC) $(CPPFLAGS) $(TARGET_ARCH) -g -c -o $@ $<
```

Why those flags specifically:
- **`$(CPPFLAGS)`** — provides `-DF_CPU=...` and (when `LIBRARY != no_lib`)
  `-I$(LIBDIR)`. The preprocessor needs these because `softserial.S` does
  `#include "registers.h"`, and `registers.h` uses `_SFR_IO_ADDR(...)` which
  expands to numeric I/O addresses derived from `<avr/io.h>`.
- **`$(TARGET_ARCH)`** — `-mmcu=attiny13a`. Selects the correct device
  definitions.
- **`-g`** — debug info, parallel to what C objects get (the gcc driver
  passes the right options on to the assembler when the input is `.S`).
- **No `$(CFLAGS)`** — those flags (`-std=gnu99`, `-Wundef`, `-fpack-struct`,
  etc.) are C-only and either meaningless or harmful for assembly.

> **Indentation note:** the leading whitespace on a make recipe line must be
> a real tab character, not spaces. If you copy-paste this and the rule
> silently doesn't fire, that's almost always the cause.

---

## Step 8 — Build a new C example that uses the asm primitives

To avoid disturbing `examples/softserial/`, put the new code under
`examples/softserial_asm/`. Following the project's "each C example is
self-contained" convention, copy the asm file and header into the example
folder rather than referencing them from `Library/`.

### Folder layout

```
examples/softserial_asm/
├── Makefile          # standard 2-line include
├── main.c            # application code
├── softserial.S      # copied from Library/ (with Step 2/3 edits)
└── softserial_asm.h  # the header you wrote in Step 6
```

### Files

**`Makefile`** (the standard 2-liner):
```makefile
DEPTH = ../../
include $(DEPTH)Makefile
```

**`main.c`** — a minimal echo program to start with:
```c
#include "softserial_asm.h"

int main(void)
{
    init_serial();
    for (;;) {
        uint8_t c = char_read();
        char_write(c);
    }
}
```

**`softserial.S`** — a copy of `Library/softserial.S` with the Step 2/3
edits applied.

**`softserial_asm.h`** — the header from Step 6.

### `env.make` requirement

The example needs `LIBRARY = no_lib` (the project-wide default for ATtiny13A
work) so the build picks up only what's in the example directory. Confirm
your `env.make` has:

```makefile
LIBRARY = no_lib
```

---

## Step 9 — Build, flash, verify

```bash
cd examples/softserial_asm
make complete
make flash
```

Inspect the build output and confirm:
- A line like `avr-gcc ... -c -o softserial.o softserial.S` appears (proves
  the new pattern rule fired).
- Final flash size is small — a few hundred bytes; the asm primitives are
  tiny.

Then connect the USB-to-serial adapter and verify in a terminal:
- `init_serial()` runs cleanly (no garbage on startup).
- Typed characters echo back exactly.
- No bit drops at sustained typing speed.

If reception is unreliable, see Step 10.

---

## Step 10 — Tuning timing if needed

The `period` and `half_period` constants in `softserial.S` are CPU-cycle
counts for the bit period:

```c
#define period       35      // # of ticks for 1 bit period (9600 baud @ 1.2 MHz)
#define half_period  18      // # of ticks for 0.5 bit period
```

`period = 35` was tuned for an ATtiny13A at the factory 1.2 MHz RC clock. If
your clock is detuned (the internal RC oscillator has ±10 % tolerance
uncalibrated), the effective baud will shift. Two avenues:

1. **Calibrate the chip** — set `OSCCAL` at startup to nudge the oscillator
   toward true 1.2 MHz. The `examples/osccal/` example helps find the right
   value for your individual chip.
2. **Re-tune `period`** — measure the bit period on a logic analyser and
   scale `period` proportionally. Each unit of `period` is 3 CPU cycles
   (`dec; brne` taken).

Do not mix the two — pick one and stick with it for repeatability.

---

## Reference — change summary

| File | Change |
|---|---|
| `Library/softserial.S` | `mov r18, r17` → `mov r18, r24`; `ror r17` → `ror r24`; drop final T-flag block; add `.global init_serial` / `.global char_write` / `.global char_read`; update header comments |
| `asm_examples/softserial/main.S` | Remove `brts main_loop`; update r17 → r24 in the register-usage comment |
| `Makefile` (root C) | Add `ASM_SOURCES` wildcard in both `LIBRARY` branches; append `$(ASM_SOURCES:.S=.o)` to `OBJECTS`; add `%.o: %.S` pattern rule |
| `examples/softserial_asm/Makefile` | New — standard 2-liner |
| `examples/softserial_asm/main.c` | New — calls `init_serial` / `char_read` / `char_write` |
| `examples/softserial_asm/softserial.S` | New — copy of edited `Library/softserial.S` |
| `examples/softserial_asm/softserial_asm.h` | New — C declarations for the three exported functions |

---

## Where to go from here

Once `char_write` and `char_read` work from C, every higher-level helper
becomes ordinary C code over those two primitives. For example:

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

All the timing-critical work is in the assembly; the C code only sequences
bytes through the primitives. Compiler-generated overhead lives in the *gap*
between characters (inside the stop bit + idle), not inside a bit period,
so it can't break the protocol.

A longer-term cleanup, once this works: have `asm_examples/softserial/main.S`
*link* `Library/softserial.S` as a separate object instead of `#include`-ing
it. Then there is one shared library object, used identically by asm and C
callers. That requires teaching `Makefile.asm` to pick up `$(LIBDIR)/*.S`
and add it to the link line — a mirror of Step 7 on the asm side. Worth
doing eventually; not required to get the C example working.
