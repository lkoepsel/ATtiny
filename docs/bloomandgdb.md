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
1. I use *VS Code* to *SSH* into the Raspberry Pi and develop my code. I use *Warp*, a terminal program to run Bloom and gdb. I've found VS Code's terminal interface to be insufficient.
