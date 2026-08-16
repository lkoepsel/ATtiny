# Per-example gdb-dashboard register selection for ATtiny13A.
# Read by ~/.gdbinit.d/avr_modules.py when avr-gdb starts in this directory,
# overriding the module defaults. See docs/gdb-dashboard.md.
#
# AVR_PERIPHERALS / AVR_BITFIELDS below cover every section produced by
# `mon rr` (see registers2.md): AC, ADC, CPU, EEPROM, EXINT, PORTB, TC0, WDT.
# FUSE and LOCKBIT are omitted -- `mon rr` reports them "inaccessible" since
# fuse/lock memory isn't part of the live target address space. GPR (r0-r31)
# is omitted from AVR_PERIPHERALS since it's already handled by AVR_REG_SET /
# AVR_REG_PAIRS below.
#
# Bit positions were pulled from avr-libc's avr/iotn13a.h (the definitive
# source for ATtiny13A SFR bit numbers), NOT from `mon rr`'s output --
# `mon rr` only reports field *values*, never bit *positions*.
# Memory addr = I/O addr + 0x20 (classic AVR SFR-to-data-space offset).

# Working registers to show, in display order. asm_blink's delay loop uses the
# temporaries r18/r19/r20 (temp_18 / temp_19 / temp_20 in Library/registers.S).
AVR_REG_SET = ["r16"]

# 16-bit pointer pairs to show as one combined value: (low, high, label).
AVR_REG_PAIRS = [("r26", "r27", "X"), ("r28", "r29", "Y"), ("r30", "r31", "Z")]

# How many working registers to pack per row.
REGS_PER_ROW = 8

# ---------------------------------------------------------------------------
#   name              addr     width
AVR_PERIPHERALS = [

    # --- AC (Analog Comparator) ---------------------------------------
    ("AC.ADCSRB",      0x0023, 1),   # ADC Control and Status Register B (ACME)
    ("AC.ACSR",        0x0028, 1),   # Analog Comparator Control and Status Register
    ("AC.DIDR0",       0x0034, 1),   # Digital Input Disable Register 0 (AIN bits)

    # --- ADC (Analog-to-Digital Converter) ------------------------------
    ("ADC.ADCSRB",     0x0023, 1),   # ADC Control and Status Register B (ADTS bits)
    ("ADC.ADC",        0x0024, 2),   # ADC Data Register (16-bit, ADCL+ADCH)
    ("ADC.ADCSRA",     0x0026, 1),   # ADC Control and Status Register A
    ("ADC.ADMUX",      0x0027, 1),   # ADC Multiplexer Selection Register
    ("ADC.DIDR0",      0x0034, 1),   # Digital Input Disable Register 0 (ADC bits)

    # --- CPU ---------------------------------------------------------
    ("CPU.PRR",        0x0045, 1),   # Power Reduction Register
    ("CPU.CLKPR",      0x0046, 1),   # Clock Prescale Register
    ("CPU.DWDR",       0x004E, 1),   # debugWire Data Register
    ("CPU.BODCR",      0x0050, 1),   # Brown-Out Detector Control Register
    ("CPU.OSCCAL",     0x0051, 1),   # Oscillator Calibration Register
    ("CPU.MCUSR",      0x0054, 1),   # MCU Status Register
    ("CPU.MCUCR",      0x0055, 1),   # MCU Control Register
    ("CPU.SPMCSR",     0x0057, 1),   # Store Program Memory Control and Status Register
    ("CPU.SPL",        0x005D, 1),   # Stack Pointer Low
    ("CPU.SREG",       0x005F, 1),   # Status Register

    # --- EEPROM --------------------------------------------------------
    ("EEPROM.EECR",    0x003C, 1),   # EEPROM Control Register
    ("EEPROM.EEDR",    0x003D, 1),   # EEPROM Data Register
    ("EEPROM.EEAR",    0x003E, 1),   # EEPROM Address Register

    # --- EXINT (External Interrupts) ------------------------------------
    ("EXINT.PCMSK",    0x0035, 1),   # Pin Change Mask Register
    ("EXINT.MCUCR",    0x0055, 1),   # MCU Control Register (ISC bits)
    ("EXINT.GIFR",     0x005A, 1),   # General Interrupt Flag Register
    ("EXINT.GIMSK",    0x005B, 1),   # General Interrupt Mask Register

    # --- PORTB ---------------------------------------------------------
    ("PORTB.PINB",     0x0036, 1),   # Input Pins, Port B
    ("PORTB.DDRB",     0x0037, 1),   # Data Direction Register, Port B
    ("PORTB.PORTB",    0x0038, 1),   # Data Register, Port B

    # --- TC0 (Timer/Counter0) -------------------------------------------
    ("TC0.GTCCR",      0x0048, 1),   # General Timer/Counter Control Register
    ("TC0.OCR0B",      0x0049, 1),   # Output Compare Register B
    ("TC0.TCCR0A",     0x004F, 1),   # Control Register A
    ("TC0.TCNT0",      0x0052, 1),   # Counter
    ("TC0.TCCR0B",     0x0053, 1),   # Control Register B
    ("TC0.OCR0A",      0x0056, 1),   # Output Compare Register A
    ("TC0.TIFR0",      0x0058, 1),   # Interrupt Flag Register
    ("TC0.TIMSK0",     0x0059, 1),   # Interrupt Mask Register

    # --- WDT (Watchdog Timer) -------------------------------------------
    ("WDT.WDTCR",      0x0041, 1),   # Watchdog Timer Control Register
]

AVR_BITFIELDS = {                  # per-bit decode for 1-byte regs

    # --- AC ---
    "AC.ADCSRB":   [(6, "ACME")],
    "AC.ACSR":     [(7, "ACD"), (6, "ACBG"), (5, "ACO"), (4, "ACI"),
                     (3, "ACIE"), (1, "ACIS1"), (0, "ACIS0")],
    "AC.DIDR0":    [(1, "AIN1D"), (0, "AIN0D")],

    # --- ADC ---
    "ADC.ADCSRB":  [(2, "ADTS2"), (1, "ADTS1"), (0, "ADTS0")],
    "ADC.ADCSRA":  [(7, "ADEN"), (6, "ADSC"), (5, "ADATE"), (4, "ADIF"),
                     (3, "ADIE"), (2, "ADPS2"), (1, "ADPS1"), (0, "ADPS0")],
    "ADC.ADMUX":   [(6, "REFS0"), (5, "ADLAR"), (1, "MUX1"), (0, "MUX0")],
    "ADC.DIDR0":   [(5, "ADC0D"), (4, "ADC2D"), (3, "ADC3D"), (2, "ADC1D")],

    # --- CPU ---
    "CPU.PRR":     [(1, "PRTIM0"), (0, "PRADC")],
    "CPU.CLKPR":   [(7, "CLKPCE"), (3, "CLKPS3"), (2, "CLKPS2"),
                     (1, "CLKPS1"), (0, "CLKPS0")],
    "CPU.BODCR":   [(1, "BODS"), (0, "BODSE")],
    "CPU.OSCCAL":  [(6, "CAL6"), (5, "CAL5"), (4, "CAL4"), (3, "CAL3"),
                     (2, "CAL2"), (1, "CAL1"), (0, "CAL0")],
    "CPU.MCUSR":   [(3, "WDRF"), (2, "BORF"), (1, "EXTRF"), (0, "PORF")],
    "CPU.MCUCR":   [(6, "PUD"), (5, "SE"), (4, "SM1"), (3, "SM0"),
                     (1, "ISC01"), (0, "ISC00")],
    "CPU.SPMCSR":  [(4, "CTPB"), (3, "RFLB"), (2, "PGWRT"),
                     (1, "PGERS"), (0, "SPMEN")],
    "CPU.SREG":    [(7, "I"), (6, "T"), (5, "H"), (4, "S"),
                     (3, "V"), (2, "N"), (1, "Z"), (0, "C")],

    # --- EEPROM ---
    "EEPROM.EECR": [(5, "EEPM1"), (4, "EEPM0"), (3, "EERIE"),
                     (2, "EEMWE"), (1, "EEWE"), (0, "EERE")],

    # --- EXINT ---
    "EXINT.MCUCR": [(1, "ISC01"), (0, "ISC00")],
    "EXINT.GIFR":  [(6, "INTF0"), (5, "PCIF")],
    "EXINT.GIMSK": [(6, "INT0"), (5, "PCIE")],

    # --- PORTB ---
    "PORTB.PINB":  [(5, "PINB5"), (4, "PINB4"), (3, "PINB3"),
                     (2, "PINB2"), (1, "PINB1"), (0, "PINB0")],
    "PORTB.DDRB":  [(5, "DDB5"), (4, "DDB4"), (3, "DDB3"),
                     (2, "DDB2"), (1, "DDB1"), (0, "DDB0")],
    "PORTB.PORTB": [(5, "PORTB5"), (4, "PORTB4"), (3, "PORTB3"),
                     (2, "PORTB2"), (1, "PORTB1"), (0, "PORTB0")],

    # --- TC0 ---
    "TC0.GTCCR":   [(7, "TSM"), (0, "PSR10")],
    "TC0.TCCR0A":  [(7, "COM0A1"), (6, "COM0A0"), (5, "COM0B1"),
                     (4, "COM0B0"), (1, "WGM01"), (0, "WGM00")],
    "TC0.TCCR0B":  [(7, "FOC0A"), (6, "FOC0B"), (3, "WGM02"),
                     (2, "CS02"), (1, "CS01"), (0, "CS00")],
    "TC0.TIFR0":   [(3, "OCF0B"), (2, "OCF0A"), (1, "TOV0")],
    "TC0.TIMSK0":  [(3, "OCIE0B"), (2, "OCIE0A"), (1, "TOIE0")],

    # --- WDT ---
    "WDT.WDTCR":   [(7, "WDTIF"), (6, "WDTIE"), (5, "WDP3"), (4, "WDCE"),
                     (3, "WDE"), (2, "WDP2"), (1, "WDP1"), (0, "WDP0")],
}

# SRAM regions to hexdump: (start_addr, length [, "label"]). Addresses are
# datasheet DATA-space; ATtiny13A SRAM is 0x0060-0x009F (RAMEND 0x9F). The
# AvrSram module (avr_modules.py) shows addr + 16 hex bytes + ASCII per row and
# is auto-added to the layout when this list is non-empty. Dump the start of
# SRAM here; point at a .data/.bss label's address instead once you add one.
# Use `avr-nm -n main.elf | grep NAME` to determine address locations
# Or `avr-nm -n main.elf ` to view all SRAM at the end
AVR_SRAM = [(0x0060, 0x40, "SRAM")]
