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
Commands which load immediate such as **`ldi` (Load Immediate) only work on R16–R31.** You cannot load a constant directly into R0–R15. As well as `andi`, `ori`, `subi`, `sbci`, and `cpi` — all immediate-operand instructions are upper-register only. If you need a constant in a low register, you must load it into an upper register first, then copy it down with `mov`.

```asm
ldi   r16, 42       ; OK
ldi   r8,  42       ; ILLEGAL — assembler error
```


### X Pointer (R26:R27) 
### Y Pointer (R28:R29)
### Z Pointer (R30:R31) 

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

## Summary

The AVR register file *looks* uniform but is actually stratified into roughly four tiers:

1. **Fully constrained** — R0, R1 (implicit hardware targets)
2. **Partially constrained** — R2–R15 (no immediate loads)
3. **Full-featured** — R16–R31 (all instructions available)
4. **Pointer registers** — X, Y, Z (addressing modes layer on top)

## ATtiny13A Register Names and Addresses for avr_dashboard.py

```bash
>>> mon rr
Reading "AC" peripheral registers...

`ac`, `adcsrb`, "ADCSRB", 0x00000023, 8-bit | 0x00 (0, 0b00000000), ACME: 0b0
`ac`, `acsr`, "ACSR", 0x00000028, 8-bit | 0x30 (48, 0b00110000), ACIS: 0b00, ACIE: 0b0, ACI: 0b1, ACO: 0b1, ACBG: 0b0, ACD: 0b0
`ac`, `didr0`, "DIDR0", 0x00000034, 8-bit | 0x00 (0, 0b00000000), AIN0D: 0b0, AIN1D: 0b0

Reading "ADC" peripheral registers...

`adc`, `adcsrb`, "ADCSRB", 0x00000023, 8-bit | 0x00 (0, 0b00000000), ADTS: 0b000
`adc`, `adc`, "ADC", 0x00000024, 16-bit | 0x0000 (0, 0b0000000000000000)
`adc`, `adcsra`, "ADCSRA", 0x00000026, 8-bit | 0x00 (0, 0b00000000), ADPS: 0b000, ADIE: 0b0, ADIF: 0b0, ADATE: 0b0, ADSC: 0b0, ADEN: 0b0
`adc`, `admux`, "ADMUX", 0x00000027, 8-bit | 0x00 (0, 0b00000000), MUX: 0b00, ADLAR: 0b0, REFS0: 0b0
`adc`, `didr0`, "DIDR0", 0x00000034, 8-bit | 0x00 (0, 0b00000000), ADC1D: 0b0, ADC3D: 0b0, ADC2D: 0b0, ADC0D: 0b0

Reading "CPU" peripheral registers...

`cpu`, `prr`, "PRR", 0x00000045, 8-bit | 0x00 (0, 0b00000000), PRADC: 0b0, PRTIM0: 0b0
`cpu`, `clkpr`, "CLKPR", 0x00000046, 8-bit | 0x03 (3, 0b00000011), CLKPS: 0b0011, CLKPCE: 0b0
`cpu`, `dwdr`, "DWDR", 0x0000004E, 8-bit | 0x00 (0, 0b00000000)
`cpu`, `bodcr`, "BODCR", 0x00000050, 8-bit | 0x00 (0, 0b00000000), BODSE: 0b0, BODS: 0b0
`cpu`, `osccal`, "OSCCAL", 0x00000051, 8-bit | 0x62 (98, 0b01100010), OSCCAL: 0b01100010
`cpu`, `mcusr`, "MCUSR", 0x00000054, 8-bit | 0x01 (1, 0b00000001), PORF: 0b1, EXTRF: 0b0, BORF: 0b0, WDRF: 0b0
`cpu`, `mcucr`, "MCUCR", 0x00000055, 8-bit | 0x00 (0, 0b00000000), ISC0: 0b00, SM: 0b00, SE: 0b0, PUD: 0b0
`cpu`, `spmcsr`, "SPMCSR", 0x00000057, 8-bit | 0x00 (0, 0b00000000), SPMEN: 0b0, PGERS: 0b0, PGWRT: 0b0, RFLB: 0b0, CTPB: 0b0
`cpu`, `spl`, "SPL", 0x0000005D, 8-bit | 0x9F (159, 0b10011111)
`cpu`, `sreg`, "SREG", 0x0000005F, 8-bit | 0x00 (0, 0b00000000), C: 0b0, Z: 0b0, N: 0b0, V: 0b0, S: 0b0, H: 0b0, T: 0b0, I: 0b0
`gpr`, `r0`, "R0", 0x00000000, 8-bit | 0x20 (32, 0b00100000)
`gpr`, `r1`, "R1", 0x00000001, 8-bit | 0xE7 (231, 0b11100111)
`gpr`, `r2`, "R2", 0x00000002, 8-bit | 0xFF (255, 0b11111111)
`gpr`, `r3`, "R3", 0x00000003, 8-bit | 0x47 (71, 0b01000111)
`gpr`, `r4`, "R4", 0x00000004, 8-bit | 0x24 (36, 0b00100100)
`gpr`, `r5`, "R5", 0x00000005, 8-bit | 0x00 (0, 0b00000000)
`gpr`, `r6`, "R6", 0x00000006, 8-bit | 0x5A (90, 0b01011010)
`gpr`, `r7`, "R7", 0x00000007, 8-bit | 0xF4 (244, 0b11110100)
`gpr`, `r8`, "R8", 0x00000008, 8-bit | 0x00 (0, 0b00000000)
`gpr`, `r9`, "R9", 0x00000009, 8-bit | 0x00 (0, 0b00000000)
`gpr`, `r10`, "R10", 0x0000000A, 8-bit | 0xFF (255, 0b11111111)
`gpr`, `r11`, "R11", 0x0000000B, 8-bit | 0x76 (118, 0b01110110)
`gpr`, `r12`, "R12", 0x0000000C, 8-bit | 0xBF (191, 0b10111111)
`gpr`, `r13`, "R13", 0x0000000D, 8-bit | 0x7D (125, 0b01111101)
`gpr`, `r14`, "R14", 0x0000000E, 8-bit | 0xF0 (240, 0b11110000)
`gpr`, `r15`, "R15", 0x0000000F, 8-bit | 0xF9 (249, 0b11111001)
`gpr`, `r16`, "R16", 0x00000010, 8-bit | 0x9F (159, 0b10011111)
`gpr`, `r17`, "R17", 0x00000011, 8-bit | 0x50 (80, 0b01010000)
`gpr`, `r18`, "R18", 0x00000012, 8-bit | 0x05 (5, 0b00000101)
`gpr`, `r19`, "R19", 0x00000013, 8-bit | 0xE3 (227, 0b11100011)
`gpr`, `r20`, "R20", 0x00000014, 8-bit | 0xF5 (245, 0b11110101)
`gpr`, `r21`, "R21", 0x00000015, 8-bit | 0x8B (139, 0b10001011)
`gpr`, `r22`, "R22", 0x00000016, 8-bit | 0xFF (255, 0b11111111)
`gpr`, `r23`, "R23", 0x00000017, 8-bit | 0xEE (238, 0b11101110)
`gpr`, `r24`, "R24", 0x00000018, 8-bit | 0x00 (0, 0b00000000)
`gpr`, `r25`, "R25", 0x00000019, 8-bit | 0x00 (0, 0b00000000)
`gpr`, `r26`, "R26", 0x0000001A, 8-bit | 0x03 (3, 0b00000011)
`gpr`, `r27`, "R27", 0x0000001B, 8-bit | 0x01 (1, 0b00000001)
`gpr`, `r28`, "R28", 0x0000001C, 8-bit | 0x05 (5, 0b00000101)
`gpr`, `r29`, "R29", 0x0000001D, 8-bit | 0x00 (0, 0b00000000)
`gpr`, `r30`, "R30", 0x0000001E, 8-bit | 0x58 (88, 0b01011000)
`gpr`, `r31`, "R31", 0x0000001F, 8-bit | 0x00 (0, 0b00000000)

Reading "EEPROM" peripheral registers...

`eeprom`, `eecr`, "EECR", 0x0000003C, 8-bit | 0x00 (0, 0b00000000), EERE: 0b0, EEWE: 0b0, EEMWE: 0b0, EERIE: 0b0, EEPM: 0b00
`eeprom`, `eedr`, "EEDR", 0x0000003D, 8-bit | 0x00 (0, 0b00000000)
`eeprom`, `eear`, "EEAR", 0x0000003E, 8-bit | 0x00 (0, 0b00000000)

Reading "EXINT" peripheral registers...

`exint`, `pcmsk`, "PCMSK", 0x00000035, 8-bit | 0x00 (0, 0b00000000)
`exint`, `mcucr`, "MCUCR", 0x00000055, 8-bit | 0x00 (0, 0b00000000), ISC00: 0b0, ISC01: 0b0
`exint`, `gifr`, "GIFR", 0x0000005A, 8-bit | 0x00 (0, 0b00000000), PCIF: 0b0, INTF0: 0b0
`exint`, `gimsk`, "GIMSK", 0x0000005B, 8-bit | 0x00 (0, 0b00000000), PCIE: 0b0, INT0: 0b0

Reading "FUSE" peripheral registers...

`fuse`, `low`, "LOW", 0x00000000, 8-bit | inaccessible
`fuse`, `high`, "HIGH", 0x00000001, 8-bit | inaccessible

Reading "LOCKBIT" peripheral registers...

`lockbit`, `lockbit`, "LOCKBIT", 0x00000000, 8-bit | inaccessible

Reading "PORTB" peripheral registers...

`portb`, `pinb`, "PINB", 0x00000036, 8-bit | 0x20 (32, 0b00100000)
`portb`, `ddrb`, "DDRB", 0x00000037, 8-bit | 0x00 (0, 0b00000000)
`portb`, `portb`, "PORTB", 0x00000038, 8-bit | 0x00 (0, 0b00000000)

Reading "TC0" peripheral registers...

`tc0`, `gtccr`, "GTCCR", 0x00000048, 8-bit | 0x00 (0, 0b00000000), PSR10: 0b0, TSM: 0b0
`tc0`, `ocr0b`, "OCR0B", 0x00000049, 8-bit | 0x00 (0, 0b00000000)
`tc0`, `tccr0a`, "TCCR0A", 0x0000004F, 8-bit | 0x00 (0, 0b00000000), WGM0: 0b00, COM0B: 0b00, COM0A: 0b00
`tc0`, `tcnt0`, "TCNT0", 0x00000052, 8-bit | 0x00 (0, 0b00000000)
`tc0`, `tccr0b`, "TCCR0B", 0x00000053, 8-bit | 0x00 (0, 0b00000000), CS0: 0b000, WGM02: 0b0, FOC0B: 0b0, FOC0A: 0b0
`tc0`, `ocr0a`, "OCR0A", 0x00000056, 8-bit | 0x00 (0, 0b00000000)
`tc0`, `tifr0`, "TIFR0", 0x00000058, 8-bit | 0x00 (0, 0b00000000), TOV0: 0b0, OCF0A: 0b0, OCF0B: 0b0
`tc0`, `timsk0`, "TIMSK0", 0x00000059, 8-bit | 0x00 (0, 0b00000000), TOIE0: 0b0, OCIE0A: 0b0, OCIE0B: 0b0

Reading "WDT" peripheral registers...

`wdt`, `wdtcr`, "WDTCR", 0x00000041, 8-bit | 0x00 (0, 0b00000000), WDE: 0b0, WDCE: 0b0, WDP: 0b0000, WDTIE: 0b0, WDTIF: 0b0
```
