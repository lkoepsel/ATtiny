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
