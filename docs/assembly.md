# Assembly Language

Here's a clean AVR assembly skeleton for the ATtiny13A, plus the full toolchain walkthrough.

Assembly examples live in `asm_examples/`. Each example follows the same 2-line local Makefile
pattern as C examples, but includes the root `Makefile.asm` instead of `Makefile`:

```makefile
DEPTH = ../../
include $(DEPTH)Makefile.asm
```

The source file must be named `main.asm` — the shared Makefile targets that name, mirroring the
`main.c` convention used by C examples.

---

## The Program — `main.asm`

```asm
; =============================================================
; main.asm  –  Minimal AVR Assembly Skeleton
; Target : ATtiny13A (1.2 MHz default internal RC oscillator)
; Toolchain: avr-as / avr-ld  (GNU Binutils for AVR)
; =============================================================

.include "tn13Adef.inc"     ; register and bit names from Library/tn13Adef.inc

.section .vectors, "ax", @progbits
    rjmp    reset_handler       ; 0x000  RESET
    reti                        ; 0x001  INT0       - External Interrupt 0
    reti                        ; 0x002  PCINT0     - Pin Change Interrupt 0
    reti                        ; 0x003  TIM0_OVF   - Timer/Counter0 Overflow
    reti                        ; 0x004  EE_RDY     - EEPROM Ready
    reti                        ; 0x005  ANA_COMP   - Analog Comparator
    reti                        ; 0x006  TIM0_COMPA - Timer0 Compare Match A
    reti                        ; 0x007  TIM0_COMPB - Timer0 Compare Match B
    reti                        ; 0x008  WDT        - Watchdog Time-out
    reti                        ; 0x009  ADC        - ADC Conversion Complete

.section .text

reset_handler:
    ; ATtiny13A has only SPL (no SPH) — RAMEND = 0x9F fits in 8 bits
    ldi     r16, lo8(RAMEND)
    out     SPL, r16

    eor     r1, r1              ; r1 = 0  (zero register by convention)
    out     SREG, r1

    rjmp    main

main:
    ; Configure PB0 (pin 5) as OUTPUT — avoid PB5 (pin 1, RESET)
    sbi     DDRB, PB0

main_loop:
    rjmp    main_loop

.section .data
.section .bss
```

---

## Build Targets (run from within an example directory)

```bash
make compile      # Assemble only
make flash        # Assemble and upload to device
make complete     # Clean, assemble, and show size
make verbose      # Flash with full avrdude output
make disasm       # Generate assembly listing (main.lst)
make size         # Show Flash/SRAM usage
make clean        # Remove build artifacts
make show_fuses   # Read fuse values from device
make env          # Print active configuration variables
```

---

## Manual Toolchain Steps

These are the raw commands `Makefile.asm` runs — useful for understanding the pipeline.

### 1. Assemble → Object file

```bash
avr-as -mmcu=attiny13a -o main.o main.asm
```

### 2. Link → ELF executable

```bash
avr-ld -m avr25 -o main.elf main.o
```

`avr25` is the linker architecture name for the ATtiny13A (enhanced reduced-core AVR, up to 8 KB flash).

### 3. Convert ELF → Intel HEX

```bash
avr-objcopy -O ihex -R .eeprom main.elf main.hex
```

### 4. Inspect

```bash
avr-objdump -d main.elf          # human-readable disassembly
avr-objdump -Pmem-usage main.elf # Flash/SRAM usage
```

### 5. Flash

```bash
avrdude -c snap_isp -p attiny13a -P usb -U flash:w:main.hex:i
```

Replace `snap_isp` with `atmelice_isp` if using the Atmel-ICE.

---

## Key concepts

- **Vector table** — must start at address `0x0000`; the ATtiny13A has 10 vectors, each one instruction word (2 bytes) wide
- **Stack pointer init** — only `SPL` is needed (no `SPH`); SRAM is 64 bytes with RAMEND = `0x9F`, which fits in 8 bits
- **No `CALL`/`JMP`** — the ATtiny13A provides only `RCALL`/`RJMP`; the 1 KB flash is well within their ±2 KB range
- **`r16`–`r31` for `LDI`** — only the upper half of the register file accepts immediate loads; `r0`–`r15` cannot be used with `LDI`
- **Avoid PB5** — pin 1 is the RESET pin; use PB0–PB4 for GPIO
- **`r1` as the zero register** — convention keeps `r1 = 0` at all times; `eor r1,r1` establishes that on startup
