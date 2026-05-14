##########------------------------------------------------------##########
##########  System-specific Details                             ##########
##########     are contained in root-level file: env.make       ##########
##########     edit to change local/board/project parameters    ##########
##########------------------------------------------------------##########
include $(DEPTH)env.make

TARGET  = main
ARCH    = avr25

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRDUDE = avrdude

## Compile only
compile: $(TARGET).hex

## Pattern rules
$(TARGET).o: $(TARGET).asm
	avr-as -mmcu=$(MCU) -I$(DEPTH)Library -o $@ $<

$(TARGET).elf: $(TARGET).o
	avr-ld -m $(ARCH) $^ -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -S $< > $@

.PHONY: compile flash verbose disasm size complete clean show_fuses avrdude_terminal env help

flash: $(TARGET).hex $(TARGET).lst size
	@echo "use make verbose to see complete programming information"
	$(AVRDUDE) -q -q -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

verbose: $(TARGET).hex $(TARGET).lst size
	$(AVRDUDE) -v -v -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

disasm: $(TARGET).lst

size: $(TARGET).elf
	$(OBJDUMP) -Pmem-usage $<

complete: clean compile size

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).o $(TARGET).lst

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
	@echo "Source file:"     $(TARGET).asm

help:
	@echo "make compile      - assemble only"
	@echo "make flash        - assemble and upload to device"
	@echo "make complete     - clean, assemble, and show size"
	@echo "make verbose      - flash with full avrdude output"
	@echo "make disasm       - generate assembly listing"
	@echo "make size         - show Flash/SRAM usage"
	@echo "make clean        - remove build artifacts"
	@echo "make show_fuses   - read fuse values from device"
	@echo "make env          - print active configuration variables"
	@echo "make help         - print this message"
