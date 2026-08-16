# Per-example gdb-dashboard register selection for asm_blink.
# Read by ~/.gdbinit.d/avr_modules.py when avr-gdb starts in this directory,
# overriding the module defaults. See docs/gdb-dashboard.md.

# Working registers to show, in display order. asm_blink's delay loop uses the
# temporaries r18/r19/r20 (temp_18 / temp_19 / temp_20 in Library/registers.S).
AVR_REG_SET = ["r16"]

# 16-bit pointer pairs to show as one combined value: (low, high, label).
AVR_REG_PAIRS = [("r26", "r27", "X"),("r28", "r29", "Y"),("r30", "r31", "Z")]

# How many working registers to pack per row.
REGS_PER_ROW = 8

# TC0 single-mode registers (addresses from avr/io.h / the ATtiny13A datasheet).
#   name           addr     width
AVR_PERIPHERALS = [
    ("TC0.GTCCR",  0x0048, 1),   # General Timer/Counter Control Register
    ("TC0.OCR0B",  0x0049, 1),   # Output Compare Register B
    ("TC0.TCCR0A", 0x004f, 1),   # Control Register A
    ("TC0.TCNT0",  0x0052, 1),   # Control Register 
    ("TC0.TCCR0B", 0x0053, 1),   # Control Register B
    ("TC0.OCR0A",  0x0056, 1),   # Output Compare Register A
    ("TC0.TIFR0",  0x0058, 1),   # Interrupt Flag Register
    ("TC0.TIMSK0", 0x0059, 1),   # Interrupt Mask Register
    # --- PORTB ---------------------------------------------------------
    ("PORTB.PINB",     0x0036, 1),   # Input Pins, Port B
    ("PORTB.DDRB",     0x0037, 1),   # Data Direction Register, Port B
    ("PORTB.PORTB",    0x0038, 1),   # Data Register, Port B
]


AVR_BITFIELDS = {                  # per-bit decode for 1-byte regs
   "TC0.TCCR0A": [(7, "COM0A1"), (6, "COM0A0"), (5, "COM0B1"), (4, "COM0B0"), (1, "WGM01"), (0, "WGM00")],
   "TC0.TCCR0B": [(7, "FOC0A"), (6, "FOC0B"), (3, "WGM02"), (2, "CS02"), (1, "CS01"), (0, "CS00")],
    # --- PORTB ---
    "PORTB.PINB":  [(5, "PINB5"), (4, "PINB4"), (3, "PINB3"),
                     (2, "PINB2"), (1, "PINB1"), (0, "PINB0")],
    "PORTB.DDRB":  [(5, "DDB5"), (4, "DDB4"), (3, "DDB3"),
                     (2, "DDB2"), (1, "DDB1"), (0, "DDB0")],
    "PORTB.PORTB": [(5, "PORTB5"), (4, "PORTB4"), (3, "PORTB3"),
                     (2, "PORTB2"), (1, "PORTB1"), (0, "PORTB0")],
}

# SRAM regions to hexdump: (start_addr, length [, "label"]). Addresses are
# datasheet DATA-space; AVR64DD32 SRAM is 0x6000-0x7FFF (RAMEND 0x7FFF). The
# AvrSram module (avr_modules.py) shows addr + 16 hex bytes + ASCII per row and
# is auto-added to the layout when this list is non-empty. Dump the start of
# SRAM here; point at a .data/.bss label's address instead once you add one.
# Use `avr-nm -n main.elf | grep NAME` to determine address locations
# Or `avr-nm -n main.elf ` to view all SRAM at the end
AVR_SRAM = [(0x0060, 0x40, "SRAM") ]
