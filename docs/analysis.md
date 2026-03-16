# reaction/main.c — Variable Analysis

## Variable Count

### Global (1)

| Line | Variable | Type |
|------|----------|------|
| 40 | `ticks_ctr` | `volatile uint8_t` |

### `press_time()` (4)

| Line | Variable | Type |
|------|----------|------|
| 80 | `button_end` | `uint8_t` |
| 83 | `button_state` | `static uint8_t` |
| 84 | `PRESSED` | `bool` |
| 85 | `DOWN` | `bool` |

### `blink()` (1)

| Line | Variable | Type |
|------|----------|------|
| 113 | `b` | `uint8_t` (parameter) |

### `main()` (10)

| Line | Variable | Type |
|------|----------|------|
| 154 | `rand` | `volatile uint8_t` |
| 158 | `good` | `volatile uint8_t` |
| 159 | `try` | `uint8_t` |
| 163 | `led_delta` | `volatile uint8_t` |
| 174 | `ALLOW` | `volatile uint8_t` |
| 175 | `i` | `uint16_t` |
| 178 | `led_start` | `uint8_t` |
| 185 | `led_end` | `uint8_t` |
| 196 | `button_delta` | `volatile uint8_t` |
| 220 | `blinks` | `volatile uint8_t` |

**Total: 16 variables** (1 global + 15 local/parameter)

Notable: 7 are declared `volatile`, and `button_state` is `static` (persists across calls to
`press_time()`). The `#define` constants (`GREEN`, `BLUE`, `RED`, `BUTTON`, `TOLERANCE`,
`OC0A_PIN`) are preprocessor macros, not variables.

---

## RAM Location Analysis

`make size` reports only 2 bytes of RAM. This reflects statically allocated RAM (`.data` +
`.bss` sections) — stack and register usage are not reported.

### The 2 Static Bytes

| Variable | Section | Why |
|----------|---------|-----|
| `ticks_ctr` (global) | `.bss` | Global — lives for the program's lifetime |
| `button_state` (static local) | `.bss` | `static` keyword — same lifetime as global |

Both are initialized to `0`, so they land in `.bss`.

### Where the Other 14 Go at Runtime

The compiler allocates them to either **registers or stack**, depending on usage:

- **Registers first** — The ATtiny13A has 32 general-purpose registers (R0–R31). The compiler
  (`-Og`) aggressively keeps local variables in registers to avoid stack overhead. Variables like
  `i`, `led_start`, `led_end`, `PRESSED`, and `DOWN` are strong candidates for register
  allocation.

- **Stack** — Variables that don't fit in registers, or whose address is taken, get pushed/popped
  on the stack during function execution. They appear in RAM only while that function is active.

### Storage Location Summary

| Storage | Reported by `make size` | When it exists |
|---------|------------------------|----------------|
| `.bss` / `.data` | Yes | Always (entire program) |
| Stack | No | Only during function execution |
| Registers | No | Only while assigned |

With only 64 bytes of RAM on the ATtiny13A, this distinction matters. The 2 bytes reported is
the *minimum* RAM floor, but peak stack depth at runtime could be significantly higher. The `-Og`
flag helps the compiler minimize stack use by preferring register allocation.

---

## Stack Depth Analysis

### Adding a `stack` Target to the Root Makefile

Add these two lines after the `compile` target (~line 127):

```makefile
stack: CFLAGS += -fstack-usage
stack: $(TARGET).hex
```

Then from within any example directory:

```bash
make clean && make stack
```

This generates a `main.su` file in the example directory:

```
main.c:65:6:init_20Hz     2    static
main.c:77:9:press_time    6    static
main.c:113:6:blink        4    static
main.c:130:5:main        16    static
```

### Reading the Output

Each line is: `file:line:col:function  frame_bytes  qualifier`

| Qualifier | Meaning |
|-----------|---------|
| `static` | Fixed frame size, compiler can determine it exactly |
| `dynamic` | Frame varies at runtime (e.g. uses `alloca`) |
| `dynamic,bounded` | Varies but within a known bound |

### Calculating Worst-Case Stack Depth

The stack grows deeper with each nested call. For `reaction`, the deepest call chain is:

```
main() → press_time()
  16   +     6       = 22 bytes
```

Add the ISR on top, since it can interrupt at any point. The ISR pushes the status register plus
any registers it uses — typically 2–10 bytes depending on what the compiler saves. For
`TIM0_COMPA_vect`, which only increments `ticks_ctr`, estimate ~4–6 bytes.

**Worst case: ~28 bytes out of 64 bytes RAM**, leaving ~34 bytes margin. Comfortable for this
program.
