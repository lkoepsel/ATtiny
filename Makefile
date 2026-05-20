##########------------------------------------------------------##########
##########  System-specific Details                             ##########
########## 	are contained in root-level file: env.make          ##########
########## 	edit to change local/board/project parameters       ##########
##########------------------------------------------------------##########
include $(DEPTH)env.make
##########------------------------------------------------------##########
##########                  Program Locations                   ##########
##########------------------------------------------------------##########

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude
AVRDUDECONF =

##########------------------------------------------------------##########
##########                   Makefile Magic!                    ##########
##########         Summary:                                     ##########
##########             We want a .hex file                      ##########
##########        Compile source files into .elf                ##########
##########        Convert .elf file into .hex                   ##########
##########        You shouldn't need to edit below.             ##########
##########------------------------------------------------------##########

## The name of your project (without the .c)
TARGET = main
## Or name it automatically after the enclosing directory
# TARGET = $(lastword $(subst /, ,$(CURDIR)))

# Object files: will find all .c/.h files in current directory
#  and in LIBDIR.  If you have any other (sub-)directories with code,
#  you can add them in to SOURCES below in the wildcard statement.

ifeq ($(LIBRARY),no_lib)
	SOURCES=$(wildcard *.c )
	ASM_SOURCES=$(wildcard *.S )
	CPPFLAGS = -DF_CPU=$(F_CPU) -DUSB_BAUD=$(USB_BAUD) -DSOFT_BAUD=$(SOFT_BAUD)

else
    SOURCES=$(wildcard *.c $(LIBDIR)/*.c)
    ASM_SOURCES=$(wildcard *.S )
    CPPFLAGS = -DF_CPU=$(F_CPU) -DUSB_BAUD=$(USB_BAUD) -DSOFT_BAUD=$(SOFT_BAUD)   -I. \
	-I$(LIBDIR)
endif

# TODO: Confirm then delete, this appears to be deprecated with the addition of the LIBRARY ['' | no_lib] parameter
# See Note re: CPPFLAGS if using/not using LIBDIR, pick only one LIB or NO_LIB
# LIB - Uncomment if the AVR_C Library is required (default), also
# uncomment LIB below in CPPFLAGS (and comment NO_LIB)
# SOURCES=$(wildcard *.c $(LIBDIR)/*.c)

# NO_LIB - Uncomment if you wish the smallest code size and DON'T
# require AVR_C Library (and comment LIB)
# SOURCES=$(wildcard *.c )

OBJECTS=$(SOURCES:.c=.o) $(ASM_SOURCES:.S=.o)
HEADERS=$(wildcard *.h)

## Compilation options, type man avr-gcc if you're curious.

# use below to setup gdb and debugging
CFLAGS = -Og -ggdb3 -std=gnu99 -Wall -Wundef -Werror -Wno-aggressive-loop-optimizations
# added to ensure C does not use r9:r8, as it is the assembly sys_clock ticks counter (see ./docs/sysclock_regpair.md)
CFLAGS += -ffixed-r8 -ffixed-r9
# Use below to optimize size
# CFLAGS = -Os -g -std=gnu99 -Wall
## Use short (8-bit) data types
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
## Splits up object files per function
CFLAGS += -ffunction-sections -fdata-sections
# if attempting to use %S format specification (strings in progmem), uncomment next line
CFLAGS += -Wno-format
LDFLAGS = -Wl,-Map,$(TARGET).map
## Uncomment line to remove interrupt vectors for smallest code size
## LDFLAGS += -nostartfiles
## Optional, but often ends up with smaller code
LDFLAGS += -Wl,--gc-sections
# Uncomment line below to add timestamp wrapper to printf() OR
# Comment line below, if  undefined reference to `__wrap_printf'
# LDFLAGS += -Wl,--wrap=printf
## Relax shrinks code even more, but makes disassembly messy
## LDFLAGS += -Wl,--relax
## LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm  ## for floating-point printf
## LDFLAGS += -Wl,-u,vfprintf -lprintf_min      ## for smaller printf
TARGET_ARCH = -mmcu=$(MCU)

## Explicit pattern rules:
##  To make .o files from .c files
%.o: %.c $(HEADERS) Makefile
	 $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<;

## Assemble .S files (uppercase S runs the C preprocessor first)
%.o: %.S Makefile
	$(CC) $(CPPFLAGS) $(TARGET_ARCH) -g -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LDLIBS) -o $@

%.hex: %.elf
	 $(OBJCOPY) -j .text -j .data -O ihex $< $@

%.eeprom: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

## These targets don't have files named after them
.PHONY: all disassemble disasm eeprom size clean clean_all squeaky_clean flash fuses


complete: all_clean compile size

compile: $(TARGET).hex

stack: CFLAGS += -fstack-usage
stack: $(TARGET).hex

static:
	cppcheck --std=c99 --platform=avr8 --enable=all --suppressions-list=$(DEPTH)suppressions.txt . 2> cppcheck.txt

env:
	@echo "MCU:"  $(MCU)
	@echo "SERIAL:"  $(SERIAL)
	@echo "F_CPU:" $(F_CPU)
	@echo "USB_BAUD:"  $(USB_BAUD)
	@echo "LIB_DIR:"  $(LIBDIR)
	@echo "LIBRARY:"  $(LIBRARY)
	@echo "PROGRAMMER_TYPE:"  $(PROGRAMMER_TYPE)
	@echo "PROGRAMMER_ARGS:"  $(PROGRAMMER_ARGS)
	@echo
	@echo "Source files:"   $(SOURCES)
	@echo

help:
	@echo "make compile - compile only (Arduino verify)"
	@echo "make flash - show program size and flash to board (Arduino upload)"
	@echo "make clean - delete all non-source files in folder"
	@echo "make clean_all - run clean in every examples/ folder"
	@echo "make complete - delete all .o files in folder & Library then compile, for complete rebuild/upload"
	@echo "make verbose - make flash with more programming information for debugging upload"
	@echo "make env - print active env.make variables"
	@echo "make size - print size information of elf file"
	@echo "make help - print this message"

# Optionally create listing file from .elf
# This creates approximate assembly-language equivalent of your code.
# Useful for debugging time-sensitive bits,
# or making sure the compiler does what you want.
disassemble: $(TARGET).lst

disasm: disassemble

# Optionally show how big the resulting program is
size:  $(TARGET).elf
# 	$(AVRSIZE) -G --mcu=$(MCU) $(TARGET).elf
	$(OBJDUMP) -Pmem-usage $(TARGET).elf
clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).obj \
	$(TARGET).o $(TARGET).d $(TARGET).eep $(TARGET).lst \
	$(TARGET).lss $(TARGET).sym $(TARGET).map $(TARGET)~ \
	$(TARGET).eeprom *.o cppcheck.txt

all_clean:
	rm -f *.elf *.hex *.obj *.o *.d *.eep *.lst *.lss *.sym *.map *~ *.eeprom core $(LIBDIR)/*.o

## Run clean in every example folder under examples/
clean_all:
	@for d in $(DEPTH)examples/*/; do \
		if [ -f "$$d"Makefile ] || [ -f "$$d"makefile ]; then \
			$(MAKE) --no-print-directory -C "$$d" clean; \
		else \
			echo "skipping $$d (no makefile)"; \
		fi; \
	done

##########------------------------------------------------------##########
##########              Programmer-specific details             ##########
##########           Flashing code to AVR using avrdude         ##########
##########------------------------------------------------------##########

flash: $(TARGET).hex $(TARGET).lst size
	@echo "use make verbose to see complete programming information"
	$(AVRDUDE) -q -q $(AVRDUDECONF) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

verbose: $(TARGET).hex $(TARGET).lst size
	$(AVRDUDE) -v -v $(AVRDUDECONF) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

## An alias
program: flash

flash_eeprom: $(TARGET).eeprom
	$(AVRDUDE) $(AVRDUDECONF) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U eeprom:w:$<

avrdude_terminal:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -nt

##########------------------------------------------------------##########
##########       Fuse settings and suitable defaults            ##########
##########------------------------------------------------------##########

## Mega 48, 88, 168, 328 default values
LFUSE = 0x62
HFUSE = 0xdf
EFUSE = 0x00

## Generic
FUSE_STRING = -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

fuses:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) \
	           $(PROGRAMMER_ARGS) $(FUSE_STRING)
show_fuses:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -nv

## Called with no extra definitions, sets to defaults
set_default_fuses:  FUSE_STRING = -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m
set_default_fuses:  fuses

## Set the fuse byte for full-speed mode
## Note: can also be set in firmware for modern chips
set_fast_fuse: LFUSE = 0xE2
set_fast_fuse: FUSE_STRING = -U lfuse:w:$(LFUSE):m
set_fast_fuse: fuses

## Set the EESAVE fuse byte to preserve EEPROM across flashes
set_eeprom_save_fuse: HFUSE = 0xD7
set_eeprom_save_fuse: FUSE_STRING = -U hfuse:w:$(HFUSE):m
set_eeprom_save_fuse: fuses

## Clear the EESAVE fuse byte
clear_eeprom_save_fuse: FUSE_STRING = -U hfuse:w:$(HFUSE):m
clear_eeprom_save_fuse: fuses
