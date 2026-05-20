##########------------------------------------------------------##########
##########  System-specific Details                             ##########
##########     are contained in root-level file: env.make       ##########
##########     edit to change local/board/project parameters    ##########
##########------------------------------------------------------##########
include $(DEPTH)env.make

## Repo-relative paths (not per-developer). Same value on every machine —
## env.make is for things that genuinely vary by machine.
LIBDIR = $(DEPTH)Library

TARGET  = main

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude

## Sources: the example's own .S files plus any extras listed in ASM_LIBS
## (set by the example's local Makefile)
SOURCES_S = $(wildcard *.S) $(ASM_LIBS)
OBJECTS   = $(SOURCES_S:.S=.o)
DEPS      = $(OBJECTS:.o=.d)

## Compile only
compile: $(TARGET).hex

## Pattern rules
%.o: %.S
	avr-gcc -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(LIBDIR) -g -Wa,--gdwarf-2 -MMD -MP -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	avr-gcc -mmcu=$(MCU) -nostartfiles -nostdlib -o $@ $^

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -S $< > $@

## Auto-generated header dependencies (one .d per .o, courtesy of -MMD -MP).
## The leading dash makes it silent on first build, before the .d files exist.
-include $(DEPS)

.PHONY: compile flash verbose disasm size complete clean clean_all show_fuses avrdude_terminal env help

flash: $(TARGET).hex $(TARGET).lst size
	@echo "use make verbose to see complete programming information"
	$(AVRDUDE) -q -q -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

verbose: $(TARGET).hex $(TARGET).lst size
	$(AVRDUDE) -v -v -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

disasm: $(TARGET).lst

size: $(TARGET).elf
# objdump -Pmem-usage reports "Device: Unknown" for hand-assembled ELFs
# (no .note.gnu.avr.deviceinfo section); avr-size knows the device itself.
	$(AVRSIZE) -C --mcu=$(MCU) $<

complete: clean compile size

clean:
	rm -f $(OBJECTS) $(DEPS) $(TARGET).elf $(TARGET).hex $(TARGET).lst

## Run clean in every example folder under asm_examples/
clean_all:
	@for d in $(DEPTH)asm_examples/*/; do \
		if [ -f "$$d"Makefile ] || [ -f "$$d"makefile ]; then \
			$(MAKE) --no-print-directory -C "$$d" clean; \
		else \
			echo "skipping $$d (no makefile)"; \
		fi; \
	done

##########------------------------------------------------------##########
##########              Programmer-specific details             ##########
##########------------------------------------------------------##########

show_fuses:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -nv

avrdude_terminal:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -nt

env:
	@echo "MCU:"             $(MCU)
	@echo "F_CPU:"           $(F_CPU)
	@echo "PROGRAMMER_TYPE:" $(PROGRAMMER_TYPE)
	@echo "PROGRAMMER_ARGS:" $(PROGRAMMER_ARGS)
	@echo
	@echo "Source file:"     $(TARGET).S

help:
	@echo "make compile      - assemble only"
	@echo "make flash        - assemble and upload to device"
	@echo "make complete     - clean, assemble, and show size"
	@echo "make verbose      - flash with full avrdude output"
	@echo "make disasm       - generate assembly listing"
	@echo "make size         - show Flash/SRAM usage"
	@echo "make clean        - remove build artifacts"
	@echo "make clean_all    - run clean in every asm_examples/ folder"
	@echo "make show_fuses   - read fuse values from device"
	@echo "make env          - print active configuration variables"
	@echo "make help         - print this message"
