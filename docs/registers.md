# AVR Register Conventions

The AVR has 32 general-purpose 8-bit registers (R0–R31), but "general-purpose" is somewhat misleading — both hardware constraints and software conventions carve out specific roles for many of them.

Additional information on register usage can be found on page 4 of "*Atmel AT1886: Mixing Assembly and C with AVRGCC [APPLICATION NOTE]*"
---

## The Full Register Map

```
R0        │ Implicit result register (hardware)
R1        │ Zero register (ABI convention)
R2–R17    │ Call-saved general purpose (ABI)
R18–R25   │ Call-clobbered general purpose (ABI)
R26:R27   │ X pointer register pair (hardware)
R28:R29   │ Y pointer register pair / frame pointer (hardware + ABI)
R30:R31   │ Z pointer register pair / indirect calls (hardware)
```

---

## Hardware-Constrained Registers

These restrictions are baked into the silicon — the instruction set physically won't let you use them otherwise.

### R0 — Implicit Result Register
The `mul`/`muls`/`mulsu` (multiply) instructions **always** write their 16-bit result into R1:R0, regardless of what you want. You have no choice. Also, the `lpm` instruction (Load Program Memory — reading from Flash) loads into R0 by default when no destination is specified.

```asm
mul   r18, r19      ; result ALWAYS goes to R1:R0, not your choice
lpm                 ; loads Flash byte into R0 (implicit form)
```

### R16–R31 — The "Upper" Registers
This is one of the most important hardware constraints for beginners. **`ldi` (Load Immediate) only works on R16–R31.** You cannot load a constant directly into R0–R15.

```asm
ldi   r16, 42       ; OK
ldi   r8,  42       ; ILLEGAL — assembler error
```

This also affects `andi`, `ori`, `subi`, `sbci`, and `cpi` — all immediate-operand instructions are upper-register only. If you need a constant in a low register, you must load it into an upper register first, then copy it down with `mov`.

### R26:R27 — The X Pointer
### R28:R29 — The Y Pointer
### R30:R31 — The Z Pointer

These three register pairs form 16-bit address pointers used for indirect memory access. The hardware instructions `ld`/`st` (load/store) use them to address SRAM, and `lpm`/`spm` use Z to address Flash.

```asm
; Load from SRAM address held in Z
ldi   r30, lo8(my_array)    ; Z low byte
ldi   r31, hi8(my_array)    ; Z high byte
ld    r16, Z+               ; load byte, post-increment Z

; All three support X±, Y±, Z± (pre/post increment/decrement)
ld    r16, X                ; indirect load via X
ld    r16, Y+               ; load, then Y++
ld    r16, -Z               ; --Z, then load
```

Y and Z also support **displacement addressing** (`ldd`/`std`), which X does not:
```asm
ldd   r16, Y+6              ; load from address Y+6 (Y unchanged)
```

This makes Y the natural **frame pointer** for accessing local variables on the stack, which the ABI formalizes.

---

## ABI / Compiler Conventions (avr-gcc)

These aren't hardware rules — they're agreements that make C and assembly interoperate correctly.

### R1 — The Zero Register
As seen in your blink program, R1 is **always expected to contain 0** by the compiler. If you use `mul` (which clobbers R1), you must restore it to zero afterward:

```asm
mul   r18, r19
; R1:R0 now hold result — R1 is no longer zero!
eor   r1, r1            ; restore the zero register
```

Forgetting this is a classic AVR bug — C code compiled by avr-gcc will silently produce wrong results if R1 isn't zero when it's called.

### Function Call Registers

**Return values** are passed back in:
| Size | Registers |
|---|---|
| 8-bit | R24 |
| 16-bit | R25:R24 |
| 32-bit | R25:R22 |
| 64-bit | R25:R18 |

**Function arguments** are passed in R25:R8 (right-to-left, 2 registers per argument for alignment), with any overflow going on the stack.

### Call-Clobbered vs. Call-Saved

**Call-clobbered (R18–R27, R30–R31)** — a called function can freely destroy these. The *caller* must save them if it needs the values after the call.

**Call-saved (R2–R17, R28–R29)** — a called function *must* preserve these. If a function uses them, it must push them on entry and pop them on exit.

```asm
my_function:
    push  r28           ; R28 is call-saved — must preserve it
    push  r29
    ; ... use R28:R29 as frame pointer ...
    pop   r29           ; restore before returning
    pop   r28
    ret
```

This mirrors the callee-saved / caller-saved convention your students see in LC-3 (where R6/R7 have special roles) and in C calling conventions generally.

---

## Practical Summary Table

| Register(s) | Hardware Role | ABI/Convention Role |
|---|---|---|
| R0 | `mul` result low, `lpm` default | Freely usable but volatile |
| R1 | `mul` result high | **Always zero** — restore after `mul` |
| R2–R15 | General (no `ldi`) | Call-saved; use `mov` to load constants |
| R16–R23 | General + `ldi` capable | Call-clobbered; workhorse temporaries |
| R24–R25 | General + `ldi` capable | **Return value & first argument** |
| R26:R27 (X) | Indirect addressing | Call-clobbered |
| R28:R29 (Y) | Indirect + displacement | **Frame pointer** — call-saved |
| R30:R31 (Z) | Indirect + Flash access | Call-clobbered |

---

## The Big Pedagogical Takeaway

The AVR register file *looks* uniform but is actually stratified into roughly four tiers:

1. **Fully constrained** — R0, R1 (implicit hardware targets)
2. **Partially constrained** — R2–R15 (no immediate loads)
3. **Full-featured** — R16–R31 (all instructions available)
4. **Pointer registers** — X, Y, Z (addressing modes layer on top)

This is a meaningful contrast with the LC-3, where all 8 registers (R0–R7) are truly symmetric from the hardware's perspective — the only asymmetry is convention (R6 as stack pointer, R7 as return address). On the AVR, the asymmetry is partly *baked into the hardware*, which is a great example of how ISA design decisions have real consequences for programmers.
