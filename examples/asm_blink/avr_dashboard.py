# Per-example gdb-dashboard register selection for asm_blink.
# Read by ~/.gdbinit.d/avr_modules.py when avr-gdb starts in this directory,
# overriding the module defaults. See docs/gdb-dashboard.md.

# Working registers to show, in display order. asm_blink's delay loop uses the
# temporaries r18/r19/r20 (temp_18 / temp_19 / temp_20 in Library/registers.S).
AVR_REG_SET = ["r16", "r17"]

# 16-bit pointer pairs to show as one combined value: (low, high, label).
AVR_REG_PAIRS = [("r26", "r27", "X"),("r28", "r29", "Y"),("r30", "r31", "Z")]

# How many working registers to pack per row.
REGS_PER_ROW = 8

# Source panel height (lines shown by `dashboard source`). Overrides the
# avr_modules.py default (10). 0 = use the whole terminal height.
AVR_SOURCE_HEIGHT = 28

#   name           addr     width
AVR_PERIPHERALS = [
    # --- PORTB ---------------------------------------------------------
    ("PORTB.PINB",     0x0036, 1),   # Input Pins, Port B
    ("PORTB.DDRB",     0x0037, 1),   # Data Direction Register, Port B
    ("PORTB.PORTB",    0x0038, 1),   # Data Register, Port B
]

AVR_BITFIELDS = {                  # per-bit decode for 1-byte regs
    # --- PORTB ---
    "PORTB.PINB":  [(5, "PINB5"), (4, "PINB4"), (3, "PINB3"), (2, "PINB2"), (1, "PINB1"), (0, "PINB0")],
    "PORTB.DDRB":  [(5, "DDB5"), (4, "DDB4"), (3, "DDB3"), (2, "DDB2"), (1, "DDB1"), (0, "DDB0")],
    "PORTB.PORTB": [(5, "PORTB5"), (4, "PORTB4"), (3, "PORTB3"), (2, "PORTB2"), (1, "PORTB1"), (0, "PORTB0")],
}

