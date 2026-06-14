# Steps to setup using Bloom, avr-gdb, ATMEL ICE, Raspberry Pi, and the ATtiny13A

## Hardware Setup
Most of the time, the debugging will be performed using debugWire which only consumes one wire (along with VCC and Ground). THe ISP interface is still required to allow Bloom to manage the fuses, hence, wire using ISP, however, development will be with debugWire.

### ATtiny13A Pinout
```
                    ATtiny13
                  ┌──────────┐
    RESET/PB5 ──1─┤          ├─8── VCC
          PB3 ──2─┤          ├─7── PB2/SCK
          PB4 ──3─┤          ├─6── PB1/MISO
          GND ──4─┤          ├─5── PB0/MOSI
                  └──────────┘
```
### Connection Table

| ISP Pin | ATtiny13A  | **13A Pin**  | Uno ISP | Color  |
|---------|------------|------------| ------- | ------ |
| RESET   | PB5/RESET  | **Pin 1**    | Pin 5   | Brown  |  
| GND     | GND        | **Pin 4**    | Pin 6   | Black  |
| MOSI    | PB0        | **Pin 5**    | Pin 4   | Green  |
| MISO    | PB1        | **Pin 6**    | Pin 1   | Yellow |
| SCK     | PB2        | **Pin 7**    | Pin 3   | Orange |
| VCC     | VCC        | **Pin 8**    | Pin 2   | Red    |


### ATtiny13A ATMEL ICE ISP Connector
Note the notch at pin 3, the arrow on the connector is incorrect.
```
                ISP Header (2x3)
              +-------+-------+
MISO/YELLOW --| 1  ●     ●  2 |--- VCC/RED
            |         |       |
SCK/ORANGE -| 3  ●       ●  4 |--- MOSI/GREEN
            |         |       |
RESET/BROWN --| 5  ●     ●  6 |--- GND/BLACK
              +-------+-------+
                  |||||||
                  ||||||+-- Pin 6: GND
                  |||||+--- Pin 5: RESET
                  ||||+---- Pin 4: MOSI
                  |||+----- Pin 3: SCK Notch/Key (orientation)
                  ||+------ Pin 2: VCC (+5V)
                  |+------- Pin 1: MISO
```

## Software Setup
1. Bloom is only available for Linux, so you need to have a development computer which runs Linux. This doesn't need to be a show-stopper. An inexpensive Raspberry Pi can be used as a headless development box, while the programming and Internet support exist on a more full function computer. In addition to Bloom, you will want to have a modern *gcc* environment, all of the details to do this are [here](./RPi_build.md).
1. I will use *VS Code* to *SSH* into the Raspberry Pi and develop my code. I use *ghostty*, a terminal program to run Bloom and gdb. I've found VS Code's terminal interface to be insufficient.

## Setup files

### bloom.yaml (ATtiny/bloom.yaml)

```yaml
environments:
  default:
    shutdown_post_debug_session: true

    tool:
      name: "xplained_mini"
 
    target:
      name: "atmega328pb"
      physical_interface: "debug_wire"
      hardware_breakpoints: true
      manage_dwen_fuse_bit: true

    server:
      name: "avr_gdb_rsp"
      ip_address: "127.0.0.1"
      port: 1442

  attiny13a:
    shutdown_post_debug_session: true

    tool:
      name: "atmel_ice"
 
    target:
      name: "attiny13a"
      physical_interface: "debug_wire"
      manage_dwen_fuse_bit: true

    server:
      name: "avr_gdb_rsp"
      ip_address: "127.0.0.1"
      port: 1442
```

### .gdbinit (~/.gdbinit)

```
set confirm off
set pagination off
set history save on
set history size 10000
set history filename ~/.gdb_history

file main.elf
target remote :1442
load
set listsize 0
set tui compact-source on
tui focus cmd

define cll
make
load
l
end

define mrc
mon reset
continue
end

define td
tui disable
end

define te
tui enable
end
```

## debugWire gotchas

Bloom manages the DWEN fuse (`manage_dwen_fuse_bit: true`), but debugWire
repurposes the **RESET pin** (PB5 on the ATtiny13A) as its single signaling
wire, and that has practical consequences you hit regularly:

1. **Chip stuck in debugWire → ISP/`make flash` fails.** While DWEN is set,
   RESET is the debugWire line, so ISP can't drive it. Bloom clears DWEN on a
   *clean* exit; a Bloom crash, a hard-killed gdb session, or pulled power
   mid-session leaves the chip in debugWire mode and `avrdude`/`make flash`
   errors out.
   **Recovery:** start another Bloom debugWire session against the chip and exit
   it cleanly — that clears DWEN and restores ISP.

2. **SNAP does not power the target.** The Microchip SNAP only senses target
   voltage (VTG); it supplies none. Power the ATtiny13A from its own supply
   before connecting, or debugWire won't come up. (The ATMEL-ICE can power the
   target, but SNAP cannot.)

3. **RESET-pin hardware.** A pull-up (~10k) on RESET is fine, but **no
   decoupling capacitor on RESET** — a cap swallows the debugWire edges and the
   connection won't establish. PB5 is also unavailable as a GPIO while debugging.

4. **Full ISP wiring still required.** Bloom uses ISP to *set* DWEN the first
   time, so the programmer needs the complete ISP connection
   (MOSI/MISO/SCK/RESET/VCC/GND), not just the RESET wire.

5. **Breakpoint budget.** Despite `hardware_breakpoints: true`, Bloom reports
   **0 available hardware breakpoints** on the SNAP + ATtiny13A (verified
   2026-06-14), so *every* breakpoint is a software breakpoint that rewrites
   flash (wear + slower). Keep breakpoint counts low.
