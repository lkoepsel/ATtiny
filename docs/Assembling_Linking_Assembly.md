## Assembling and Linking AVR Assembly Programs to ELF Format

Here's a complete guide for assembling and linking AVR assembly language programs for the ATtiny13A using avr-gcc:

### File Types and Build Process

The build process for assembly involves these file types:
- **```.S```** or **```.s```** - Assembly source file (uppercase ```.S``` allows C preprocessor directives)
- **```.o```** - Object file (assembled output)
- **```.elf```** - ELF executable (linked output for debugging)
- **```.hex```** - Intel HEX format (for flashing to the microcontroller)

### Assembly and Linking Commands

#### Step 1: Assemble the assembly source to object file
```bash
avr-gcc -g -mmcu=attiny13a -c -o main.o main.S
```

#### Step 2: Link object file to ELF executable
```bash
avr-gcc -g -mmcu=attiny13a -nostartfiles -o main.elf main.o
```

#### Step 3: (Optional) Generate HEX file for flashing
```bash
avr-objcopy -O ihex -R .eeprom main.elf main.hex
```

### Key Assembler/Linker Directives

- **```-mmcu=attiny13a```** - Specifies the target microcontroller
- **```-nostartfiles```** - Prevents linking default C runtime startup code
- **```-g```** - Include debugging information for avr-gdb
- **```-c```** - Assemble only, don't link
- **```-o```** - Specify output filename
- **```-Wa,option```** - Pass options directly to the assembler

### Sample LED Blink Program (Assembly)

Here's a complete AVR assembly program to blink an LED on PB3 (pin 2):

```assembly
; main.S - LED Blink for ATtiny13A in Assembly
.include "tn13adef.inc"    ; Include ATtiny13A definitions

.equ LED_PIN, PB3           ; LED connected to PB3

.section .text
.global main
.org 0x0000                 ; Reset vector

main:
    ; Set up stack pointer (optional for simple programs)
    ldi r16, low(RAMEND)
    out SPL, r16
    
    ; Configure PB3 as output
    sbi DDRB, LED_PIN       ; Set Data Direction Register bit
    
blink_loop:
    ; Turn LED on
    sbi PORTB, LED_PIN      ; Set PORTB bit high
    rcall delay_500ms
    
    ; Turn LED off
    cbi PORTB, LED_PIN      ; Clear PORTB bit low
    rcall delay_500ms
    
    rjmp blink_loop         ; Infinite loop

; Delay subroutine (~500ms at 1.2MHz)
delay_500ms:
    ldi r18, 41
delay_outer:
    ldi r17, 255
delay_middle:
    ldi r16, 255
delay_inner:
    dec r16
    brne delay_inner
    dec r17
    brne delay_middle
    dec r18
    brne delay_outer
    ret

.end
```

### Alternative: Minimal Assembly Without Include File

If you don't have the ```tn13adef.inc``` file, here's a minimal version with direct register addresses:

```assembly
; main.S - Minimal LED Blink for ATtiny13A
.equ DDRB, 0x17             ; Data Direction Register B
.equ PORTB, 0x18            ; Port B Data Register
.equ SPL, 0x3D              ; Stack Pointer Low
.equ RAMEND, 0x9F           ; End of RAM

.section .text
.global main
.org 0x0000

main:
    ; Configure PB3 as output
    ldi r16, 0x08           ; Bit 3 = PB3
    out DDRB, r16
    
blink:
    ; Toggle LED
    ldi r16, 0x08           ; LED on
    out PORTB, r16
    rcall delay
    
    ldi r16, 0x00           ; LED off
    out PORTB, r16
    rcall delay
    
    rjmp blink

delay:
    ldi r20, 100
d1: ldi r19, 255
d2: ldi r18, 255
d3: dec r18
    brne d3
    dec r19
    brne d2
    dec r20
    brne d1
    ret

.end
```

### Complete Build Example with Makefile

Here's a Makefile for automating the assembly process:

```makefile
MCU = attiny13a
AS = avr-gcc
LD = avr-gcc
OBJCOPY = avr-objcopy
ASFLAGS = -g -mmcu=$(MCU) -c
LDFLAGS = -g -mmcu=$(MCU) -nostartfiles

TARGET = main

all: $(TARGET).elf $(TARGET).hex

$(TARGET).elf: $(TARGET).o
	$(LD) $(LDFLAGS) -o $@ $^

$(TARGET).o: $(TARGET).S
	$(AS) $(ASFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	rm -f $(TARGET).o $(TARGET).elf $(TARGET).hex

size: $(TARGET).elf
	avr-size --format=avr --mcu=$(MCU) $(TARGET).elf

.PHONY: all clean size
```

### Building and Debugging

To build:
```bash
make
```

To check program size:
```bash
make size
```

To debug with avr-gdb:
```bash
avr-gdb main.elf
```

The ELF file contains all debugging symbols needed for avr-gdb to properly debug your assembly program. The ```-g``` flag ensures debugging information is preserved throughout the build process.

### Hardware Connection

For the LED blink example:
- Connect LED anode to PB3 (physical pin 2)
- Connect LED cathode through a 220Ω resistor to ground
- Power the ATtiny13A with 3-5V on VCC (pin 8) and GND (pin 4)

### Important Notes

- The **```-nostartfiles```** flag is crucial for pure assembly programs to prevent linking the C runtime startup code [^3][^2]
- Use uppercase ```.S``` extension if you want to use C preprocessor directives like ```#define```
- The ATtiny13A has only 1KB flash and 64 bytes RAM, so assembly is ideal for maximum control [^1]

[^1]: [Programming an ATtiny13A in Assembly - Hackster.io](https://www.hackster.io/gatoninja236/programming-an-attiny13a-in-assembly-30a529#:~:text=Programming%20an,of%20memory%21) 44%
[^2]: [c - Difference using avr-gcc and avr-ld in ELF... - Stack Overflow](https://stackoverflow.com/questions/53530449/difference-using-avr-gcc-and-avr-ld-in-elf-executable-and-linkable-format-file#:~:text=using%20avr-gcc,like%20assembly.) 41%
[^3]: [ATTiny13 -- avr-gcc Hello World uses over 100 bytes?](https://electronics.stackexchange.com/questions/2377/attiny13-avr-gcc-hello-world-uses-over-100-bytes#:~:text=gcc%20has,be%20difficult) 15%