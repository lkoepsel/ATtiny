# Programming the ATtiny13A in Standard C

Notes as to developing C code for the Microchip ATtiny13A. 

[ATtiny13A Pinout](./ATtiny13A%20Pinout.png)

## Introduction
This repository provides example programs in  [*C* (ANSI C99) AVR-LibC](https://github.com/avrdudes/avr-libc) which support programming the ATtiny13A. In order to use this framework, you can either install the *GNU avr* tool chain appropriate for your computer (*Linux*, *macOS*, or *Windows*). 

As there is not a boot loader for the *ATtiny13A* due to memory constraints, you will need to use an *ATMEL-ICE* or *Microchip SNAP* to load programs. The *env.make* allows for this, with entries for both.

 I also recommend using [Bloom](https://bloom.oscillate.io/) and *avr-gdb*. Bloom provides the ability to load code as well as display the microcontroller's registers and memory. More details specific to the *ATTiny13A*, **bloom**, and *gdb* at the [bottom of this page](#using-bloom-and-avr-gdb).

[gdb](https://www.sourceware.org/gdb/) is a simple yet extremely powerful debugging tool. I find it easier to use than most IDE's such as Visual Studio, MPLAB IDE etc. More guidance at [Developing in C for the ATmega328: Setup Bloom and gdb for Hardware Debug](https://wellys.com/posts/avr_c_gdb_bloomsetup/). 

## Local Documentation

* [ATTiny13A Datasheet](./documentation/ATtiny13A-Data-Sheet-DS40002307A.pdf)
* [Getting Started Writing C for AVR](./documentation/AVR1000b-Getting-Started-Writing-C-Code-for-AVR-DS90003262B.pdf)
* [AVR Instruction Set](./documentation/Atmel-0856-avr-instruction-set-manual.pdf)
* [AVR Software User Guide](./documentation/Atmel-42787-AVR-Software-User-Guide.pdf)
* [AVR Efficient C Coding](./documentation/Atmel-AVR035-Efficient%20C%20Coding%20for%20AVR.pdf)
* [AVR_LibC User Manual](./documentation/avr-libc-user-manual-2.2.0.pdf)
* [Avrdude User Manual](./documentation/avrdude.pdf)
* [GCC User Manual](./documentation/gcc_15.pdf)
* [GDB User Manual](./documentation/gdb_17.0.50.20250617-git.pdf)

## Online Documentation

* [Microchip ATtiny13A](https://www.microchip.com/en-us/product/attiny13a)
* [ATtiny13A-Data-Sheet-DS40002307A](https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny13A-Data-Sheet-DS40002307A.pdf) 
* [*C* (ANSI C99) AVR-LibC](https://github.com/avrdudes/avr-libc)
* [GDB Online Manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/index.html#Top) 
* [Bloom Documentation](https://bloom.oscillate.io/docs/getting-started)
* [Bloom Target Information](https://bloom.oscillate.io/docs/target/attiny13a)
* [Arduino Sensors](https://arduinomodules.info)
* [Arduino Compatible MicroCore for ATtiny](https://github.com/MCUdude/MicroCore/)

## docs

### Table of Contents

#### [env_make.md](./docs/env_make.md)
The file env_make is used to customize the *make* process for *ATtiny* development. It is **required** in order for make to properly identify the parameters needed for compiling/linking/uploading executable code to an AVR microcontroller. **This file is not tracked by *git* and needs to be installed manually.** 

#### [vs_code.md](./docs/vs_code.md)
This page contains the files needed to be more efficient with *VS Code*. Install them in the *.vscode* folder of *ATtiny*. **They are not tracked by *git*.**

#### [git.md](./docs/git.md)
Notes on using *git*. I am neither an expert on *git* nor proficient in *git*. This page is primarily for myself, however, it has helped a few people.

#### [RPI_build.md](https://github.com/lkoepsel/AVR_C/blob/main/docs/RPi_build.md) - see this  in the AVR_C repository
If you want to use a Raspberry Pi (3/4/5) as a C development platform, this page is for you! Highly detailed, it will show every step required to build the latest software for developing C on an AVR microcontroller. Have fun! 

#### [bloom and gdb.md](./docs/bloomandgdb.md)
Given the ATtiny13A requires a hardware interface to load software, I recommend using *bloom* as the interface to avr-gdb. This provides loading and debugging capability, which is required to be successful. 

#### [PWM and ADC.md](./docs/PWMandADC.md)
Page under development to explain PWM and ADC interaction. 

## Steps to Use
1. Install the AVR toolchain which consists of *avr-gcc*, *avr-gdb*, and *avrdude* as well as *make* and *git*. The **best** method is to use a [Raspberry Pi as your development platform.](./docs/RPi_build.md). If you wish to use *Windows* or *macOS*, some instruction is provided [here](https://www.wellys.com/posts/avr_c_setup/).
2. Clone this repository.
3. Open the *ATtiny* folder and add an *env.make* file (*see below*) based on your programming board and system.
4. Navigate to *examples/blink* in your CLI and run:
	* *make* to compile, link and create an executable file
	* *make flash* to upload executable file to your board.
5. Look at the other examples to better understand how to use the code.

## Programming Summary
For most of this development, I used the *Microchip SNAP* along with *bloom* and *avr-gdb*. This combination, replaces the bootloader, provides significant debugging resources and is inexpensive ([*Microchip SNAP is $12*](https://www.digikey.com/en/products/detail/microchip-technology/PG164100/9562532)).
### Note:
1. Bloom requires specific functionality of the ISP interface. Connect via ISP then use debugWire for debugging and loading programs. 
   From Bloom: "*The debugWIRE interface does not support fuse programming. Fuses can only be changed via the ISP interface (for debugWIRE AVR targets). In order for Bloom to manage the DWEN fuse bit, the debug tool must be connected to the target via the ISP interface.*"
1. **The note above means *bloom* will control the DWEN fuse. The less the fuse is changed, the better, therefore, it's best to use *load* and *mon reset* within *avr-gdb* instead of using AVRDUDE, when connected using the ATMEL ICE or Microchip SNAP.**
1. *PB3* and *PB4* are available using *ISP* programmer
1. *PB0-PB4* are available using *debugWire*. Most of the time, *bloom* is interacting with the *ATtiny13A* using the *RESET* pin (*debugWIRE*), which means you can easily use all of the *PORTB* pins.
1. The arrow on the ATMEL ICE ISP connector is incorrect. Pin 3 (middle pin) is at the notch.

## Making Connections
Insert your ATtiny13A into your breadboard. You will then need to make connections to it via a cable from the *ATMEL-ICE* or *SNAP*.

### **Minimal Working Circuit**
1. Connect **VCC** (pin 8) to your power supply (2.7-5.5V)
2. Connect **GND** (pin 4) to ground
3. Add **100nF decoupling capacitor** between VCC and GND
5. Connect ISP header for programming
6. When **NOT** using *debugWIRE*, add a 10kΩ pull-up resistor to pin 1 **RESET**, along with a button (one side to **GND**, other side to **RESET**)

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

### ISP Connections

| ISP Desc | ISP Pin   | ATtiny13A  | **13A Pin** | Color  |
|---------|--------|------------|-------------|--------|
| RESET   | Pin 5  | PB5/RESET  | **Pin 1**   | Brown  |
| GND     | Pin 6  | GND        | **Pin 4**   | Black  |
| MOSI    | Pin 4  | PB0        | **Pin 5**   | Green  |
| MISO    | Pin 1  | PB1        | **Pin 6**   | Yellow |
| SCK     | Pin 3  | PB2        | **Pin 7**   | Orange |
| VCC     | Pin 2  | VCC        | **Pin 8**   | Red    |

### ATtiny13A Connections for ATMEL-ICE Connector

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
                  |||+----- Pin 3: SCK
                  ||+------ Pin 2: VCC (+5V)
                  |+------- Pin 1: MISO
                  +-------- Notch/Key (orientation)
```

### ATtiny 13A Connections for SNAP SIL
| Snap SIL | Signal/Adapter | Wire Color | **13A Pin**  | 
| :---------: | :-----------: | :------: | :----------: |
| 1  | N/C    | none   | N/C |
| 2  | VTG   | Red    | 8   |
| 3  | GND   | Black  | 4   |
| 4  | MISO  | Yellow | 6   |
| 5  | SCK   | Orange | 7   |
| 6  | RESET | Brown  | 1   |
| 7  | MOSI  | Green  | 5   |
| 8  | N/C    | none   | N/C |

### **Modified Reset Pin Configuration for debugWIRE**
- If exists, **remove the 10kΩ pull-up resistor** from the RESET pin (pin 1)
- The RESET pin becomes a **bidirectional communication line** when DebugWire is enabled
- Keep the reset line as short as possible to minimize noise

Remember that while DebugWire provides powerful debugging capabilities with just one wire, it **temporarily disables normal ISP programming** until you explicitly disable the DebugWire mode.


## Port B Pin Functionality

| 13A PDIP | Port Pin | Alternate Functions |
| --- |----------|-------------------|
|  1  | **PB5** | • RESET: Reset Pin<br>• dW: debugWIRE I/O<br>• ADC0: ADC Input Channel 0<br>• PCINT5: Pin Change Interrupt, Source 5 |
|  3  | **PB4** | • ADC2: ADC Input Channel 2<br>• PCINT4: Pin Change Interrupt 0, Source 4 |
|  2  | **PB3** | • CLKI: External Clock Input<br>• ADC3: ADC Input Channel 3<br>• PCINT3: Pin Change Interrupt 0, Source 3 |
|  7  | **PB2** | • SCK: Serial Clock Input<br>• ADC1: ADC Input Channel 1<br>• T0: Timer/Counter0 Clock Source<br>• PCINT2: Pin Change Interrupt 0, Source 2 |
|  6  | **PB1** | • MISO: SPI Host Data Input / Client Data Output<br>• AIN1: Analog Comparator, Negative Input<br>• OC0B: Timer/Counter0 Compare Match B Output<br>• INT0: External Interrupt 0 Input<br>• PCINT1: Pin Change Interrupt 0, Source 1 |
|  5  | **PB0** | • MOSI: SPI Host Data Output / Client Data Input<br>• AIN0: Analog Comparator, Positive Input<br>• OC0A: Timer/Counter0 Compare Match A output<br>• PCINT0: Pin Change Interrupt 0, Source 0 |

## ADC Features
* 4 Channels PinB2-5
* 1 Analog Comparator PB0-1
* 10-bit resolution
* 260us conversion time (*longest*)

### **Power Supply Components**
- **VCC**: Connect to pin 8 (supply voltage: 2.7V to 5.5V)
- **GND**: Connect to pin 4
- **Decoupling capacitor**: 100nF ceramic capacitor between VCC and GND, placed as close to the chip as possible

### **Important Considerations**

1. **Fuse Configuration**

  * You must **enable the DWEN (DebugWire Enable) fuse bit**
  * Once enabled, the RESET pin loses its reset functionality and becomes a DebugWire interface
  * ISP programming is disabled when DebugWire is active
  * **Bloom does this quite well, leave it to Bloom**

2. **Debugger Requirements**
  * Use a compatible debugger like **Atmel-ICE**, **Microchip SNAP**, or **AVR Dragon**
  * The debugger must support DebugWire protocol

3. **Power Cycling**
  * After enabling DebugWire, you need to **power cycle** the target device
  * The debugger will then communicate through the single-wire interface

### **To Disable DebugWire**
If you need to return to normal ISP programming:
1. Use the debugger to **disable DebugWire** through the debugging software
1. Use *avrdude* to write a **HIGH** of 0xff to the ATtiny13A, see *avrdude commands* below

## *avrdude* commands

### LOW Fuse Reference for Clock Speed (Default in BOLD)

| Clock Speed | Low Fuse | CLKDIV8 | Oscillator |
|-------------|----------|----------| --------------- |
| 9.6MHz      | 0x7A     | Disabled    | 9.6MHz       |
| 4.8MHz      | 0x79     | Disabled    | 4.8MHz       |
| **1.2MHz**  | **0x6A** | **Enabled** | **9.6MHz÷8** |
| 600kHz      | 0x69     | Enabled     | 4.8MHz÷8     |

### HIGH Fuse Reference for DWEN (Default in BOLD)

| Mode        | HIGH Fuse| debugWIRE| 
|-------------|----------|---------------|
| **ISP**     | **0xFF** | **Disabled**  |
| *debugWIRE*   | 0x7F     | Enabled       | 


#### simple *avrdude* terminal command to test connection
```bash
avrdude -p attiny13a -P usb  -c snap_isp -t
```

* *?* for help
* *part* for confirming chip
* *q* to quit

#### read fuses, high fuse needs to be 0xff for ISP (*avrdude*) programming
```bash
avrdude -c snap_isp -p attiny13a -U lfuse:r:-:h -U hfuse:r:-:h
```

#### if required, write fuses to allow for ISP programming
```bash
avrdude -c snap_isp -p attiny13a -U lfuse:w:0x6A:m -U hfuse:w:0xFF:m
```

#### program ATtiny13A
```bash
avrdude -c snap_isp -p attiny13a -U flash:w:main.elf:e
```

#### confirm hash of FLASH memory
```bash
avrdude -p t13 -c snap_isp -U flash:r:-:i 2>/dev/null | md5sum
```

#### Confirming hex file against FLASH
```
# Verify flash against a file
avrdude -p t13 -c snap_isp -U flash:v:main.hex:i
```

## Make Commands (requires ISP interface)

* **complete**: Cleans out all .o files and recompiles
* **size**: Shows the size of program and memory
* **flash**: Uploads program to *ATtiny13A*

## Troubleshooting

**"Device signature = 0x000000" error:**
- Check wiring connections
- Verify power supply
- Try slower programming speed: ```-B 10```

**"Initialization failed" error:**
- Ensure programmer drivers are installed
- Check USB connection
- Verify programmer type in avrdude command

**LED not blinking:**
- Verify LED polarity and resistor value
- Check if correct pin is used in code
- Ensure F_CPU matches actual clock frequency

### Alternative Programmer Types for Atmel-ICE
The Atmel-ICE supports multiple programming modes:
- **ISP mode**: ```-c atmelice_isp```
- **debugWIRE**: ```-c atmelice_dw```

### Voltage Settings
The **SNAP programmer doesn't provide target power**, so you must power the ATtiny13A externally. The Atmel-ICE can provide power, but it's often better to use external power.

## Troubleshooting Specific to These Programmers

### Atmel-ICE Issues
- **"No Atmel-ICE units found"**: Check USB drivers, may need to install **libusb** on Linux
- **Connection issues**: Try adding ```-B 10``` to slow down ISP clock
- **Permission errors on Linux**: Add udev rules or run with sudo

### SNAP Issues
- **"Cannot find SNAP"**: Update to latest AVRDUDE version
- **Power issues**: Remember SNAP doesn't supply power - **external power required**
- **Slower programming**: SNAP can be slower than other programmers; this is normal

### Linux USB Permissions
For both programmers on Linux, you may need to add udev rules:

Create ```/etc/udev/rules.d/99-atmel.rules```:
```bash
# Atmel-ICE
SUBSYSTEM=="usb", ATTRS{idVendor}=="03eb", ATTRS{idProduct}=="2141", MODE="0666"

# Microchip SNAP
SUBSYSTEM=="usb", ATTRS{idVendor}=="03eb", ATTRS{idProduct}=="2180", MODE="0666"
```

Then reload rules:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Understanding the ATtiny13A Clock frequencies

The ATtiny13A has an internal RC oscillator that can run at either 4.8MHz or 9.6MHz. There is a clock divider (CLKDIV8) that is enabled by default from the factory, which divides the clock by 8. So:

- Default factory setting: 9.6MHz oscillator with CLKDIV8 enabled = 1.2MHz system clock or *1.2MHz is the default system clock frequency after the divider*

The oscillator runs at 9.6MHz but the system clock is 1.2MHz by default due to the divider.

To run at 9.6MHz, you need to:
1. Keep the oscillator at 9.6MHz (which is already the default)
2. Disable the CLKDIV8 fuse

### ATtiny13A Clock System Explanation

The **ATtiny13A** has an internal RC oscillator that runs at either:
- **9.6MHz** (default oscillator setting)
- **4.8MHz** (alternative setting)

However, there's a crucial detail: the **CLKDIV8 fuse is programmed by default** from the factory, which divides the clock by 8.

- **Default from factory**: 9.6MHz ÷ 8 = **1.2MHz system clock**

## Fuse Settings for Different Frequencies

### Low Fuse (Default: 0x6A)
| Bit | Default | Name | Description |
|-----|---------|------|-------------|
| 7 | 0 | SPIEN | SPI programming enabled (programmed) |
| 6 | 1 | EESAVE | Preserve EEPROM memory through Chip Erase |
| 5 | 1 | WDTON | Watchdog Timer always on |
| 4 | 0 | CKDIV8 | Divide clock by 8 |
| 3 | 1 | SUT1 | Start-up time |
| 2 | 0 | SUT0 | Start-up time |
| 1 | 1 | CKSEL1 | Select clock source |
| 0 | 0 | CKSEL0 | Select clock source |

### To Run at 9.6MHz System Clock

```bash
## write CLKDIV8 (bit 4) a 1 meaning unprogrammed, in this case DWEN (bit 3 H fuse) was already programmed
avrdude -c snap_isp -p attiny13a -U lfuse:w:0x7A:m -U hfuse:w:0xF7:m
```

### To Run at 1.2MHz System Clock (Factory Default)
This command will also "deprogram" or "set" the DWEN (*Hfuse: Bit 3*) nfuse, which will allow the RESET pin to operate properly. Bloom will clear the bit, which puts the ATtiny into DEBUG mode.

```bash
avrdude -c snap_isp -p attiny13a -U lfuse:w:0x6A:m -U hfuse:w:0xFF:m
```
### To Confirm or See Fuse Values

```bash
# read the fuses back
avrdude -c snap_isp -p attiny13a -U lfuse:r:-:h -U hfuse:r:-:h
# be sure to set CPU speed in env.make to 9600000UL
```

### Compile for 9.6MHz

Update the Makefile:
```makefile
F_CPU = 9600000UL  # 9.6MHz instead of 1200000UL
```

**Default factory values:**
- Low Fuse: **0x6A**
- High Fuse: **0xFF**

### Complete Fuse Reference for Clock Speed

| Clock Speed | Low Fuse | CLKDIV8 | Oscillator |
|-------------|----------|----------|------------|
| 9.6MHz      | 0x7A     | Disabled | 9.6MHz     |
| 4.8MHz      | 0x79     | Disabled | 4.8MHz     |
| 1.2MHz      | 0x6A     | Enabled  | 9.6MHz÷8   |
| 600kHz      | 0x69     | Enabled  | 4.8MHz÷8   |

## Important Notes

1. **F_CPU must match actual clock**: If you set fuses for 9.6MHz but compile with F_CPU=1200000UL, delays will be **8 times faster** than expected

2. **Power consumption**: Higher clock speeds consume more power

3. **Voltage requirements**: At higher frequencies, ensure adequate voltage (see datasheet for speed grades)

4. **After changing fuses**: The new clock speed takes effect immediately, so subsequent programming operations will use the new speed

The confusion often arises because Atmel/Microchip ships the chips with **CLKDIV8 enabled by default** for compatibility and lower power consumption, even though the oscillator itself runs at 9.6MHz.

## Multiplex ATtiny13A pin
### Context Provided
I wish to multiplex an AVR ATtiny13A pin. Provide a circuit example where the microcontroller has a pin being used as both input and output. For example, I want to measure temperature with a thermistor then light an LED based on the thermistor value, with both the thermistor and led will be on the same pin.  Explain via a circuit analysis, that the thermistor won’t be affected when the LED is lit and the LED won’t light when the thermistor is being measured via the ADC of the ATtiny13A pin.

### Claude 4 Response
To multiplex a pin on the ATtiny13A for both input (thermistor measurement) and output (LED control), you can use a clever circuit design that ensures the thermistor and LED do not interfere with each other. Below is an example circuit and explanation of how it works.

---

### **Circuit Design**
1. **Components**:
   - **Thermistor**: Connected in a voltage divider configuration with a resistor.
   - **LED**: Connected in series with a current-limiting resistor.
   - **ATtiny13A Pin**: Configured as an ADC input for the thermistor and as an output to drive the LED.

The 74HC4053 is a triple 2:1 analog multiplexer/demultiplexer. It has three independent switches, each with two inputs and one output. The user only needs one of these switches for their application.

## **What the 74HC4053 Is:**
It contains **three independent 2-to-1 switches** (like three SPDT switches). One switch is required for this application.

## **Simplified Connection for the Circuit:**

```
                    74HC4053
                 +------------+
Thermistor-R1 ---|Y0       COM|--- To ADC Pin
                 |            |
LED-R2 ----------|Y1        S |--- To GPIO (Select)
                 |            |
GND -------------|INH      VCC|--- 5V
                 |            |
GND -------------|VEE      GND|--- GND
                 +------------+
```

## **How It Works:**

**Pin Functions:**
- **Y0, Y1**: The two inputs being switched
- **COM**: Common output (connects to either Y0 or Y1)
- **S**: Select pin (LOW = Y0, HIGH = Y1)
- **INH**: Inhibit (tie to GND to enable the switch)
- **VEE**: Negative supply (tie to GND for single supply)

**Operation:**
- When **S = LOW**: COM connects to Y0 (thermistor circuit)
- When **S = HIGH**: COM connects to Y1 (LED circuit)

## **Your Complete Circuit:**
```
    5V                          5V
     |                           |
Thermistor                      VCC
     |                           |
     +----------Y0          74HC4053
     |           |               |
   [10k]        COM-----ADC Pin  |
     |           |               |
    GND         Y1               |
                 |               |
                LED              |
                 |               |
              [330Ω]             |
                 |               |
                GND              |
                                 |
    GPIO Pin----S                |
                                 |
    GND--------INH, VEE, GND-----+
```


## **CD74HCT4053B Complete Pinout**

* [TI CD74HCT4053B Datasheet](https://www.ti.com/lit/ds/symlink/cd74hct4053.pdf?ts=1749252478329&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FCD74HCT4053)
* [TI CD74HCT4053](https://www.ti.com/product/CD74HCT4053)
```
                  CD74HCT4053B (16-pin PDIP)
                    +--------U--------+
       LED ---------|1  B1      VCC 16|------- 5V
                    |                 |
   Thermistor/10k --|2  B0       A1 15|------- NC
                    |                 |
              NC ---|3  C1       A0 14|------- NC
                    |                 |
              NC ---|4  C0    COM A 13|------- NC
                    |                 |
              NC ---|5  COM C COM B 12|------- ADC Pin
                    |                 |
             GND ---|6  INH      SA 11|------- NC
                    |                 |
             GND ---|7  VEE      SB 10|------- GPIO (Select)
                    |                 |
             GND ---|8  GND      SC  9|------- NC
                    +-----------------+
```

## **Pin Connections Summary:**

| Pin | Name | Connection |
|-----|------|------------|
| 1   | B1   | **LED anode** |
| 2   | B0   | **Thermistor/10k junction** |
| 3   | C1   | NC |
| 4   | C0   | NC |
| 5   | COM C| NC |
| 6   | INH  | **GND** (enables all switches) |
| 7   | VEE  | **GND** (for single supply) |
| 8   | GND  | **GND** |
| 9   | SC   | NC |
| 10  | SB   | **GPIO Select Pin** |
| 11  | SA   | NC |
| 12  | COM B| **ADC Pin** |
| 13  | COM A| NC |
| 14  | A0   | NC |
| 15  | A1   | NC |
| 16  | VCC  | **5V** |

**Control Logic:**
- When **SB = LOW**: COM B connects to B0 (thermistor)
- When **SB = HIGH**: COM B connects to B1 (LED)

**Notes:**
- Using only the B channel (pins 1, 2, 10, 12)
- Channels A and C are unused
- INH must be LOW to enable operation


**Software Control:**
```c
// Read temperature
digitalWrite(SELECT_PIN, LOW);  // Connect thermistor
int temp = analogRead(ADC_PIN);

// Control LED
digitalWrite(SELECT_PIN, HIGH); // Connect LED
pinMode(ADC_PIN, OUTPUT);
digitalWrite(ADC_PIN, HIGH);    // LED on
```

**Key Benefits:**
- **Complete isolation** between circuits
- **No voltage limitations**
- **No power waste**
- **Bidirectional** - can pass analog or digital signals

## Additional Elements 

* [CR2025 Vs CR2032 Batteries – Are They Interchangeable? | Battery Tools](https://batterytools.net/what-is-the-difference-between-2032-and-2025-batteries/)
* [Lithium Ion Polymer Battery with Short Cable - 3.7V 420mAh : ID 4236 : Adafruit Industries, Unique & fun DIY electronics and kits](https://www.adafruit.com/product/4236)
* [Overview | Sipping Power With NeoPixels | Adafruit Learning System](https://learn.adafruit.com/sipping-power-with-neopixels)
* [NeoPixel RGBW Mini Button PCB - Pack of 10 : ID 4776 : Adafruit Industries, Unique & fun DIY electronics and kits](https://www.adafruit.com/product/4776)
* [Using the RESET pin on Startup](https://sites.google.com/site/wayneholder/using-the-avr-reset-pin-as-an-input)
## Helpful Files

### tasks.json for VS Code

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "make",
            "detail": "Run make",
            "type": "shell",
            "command": "/usr/bin/make ${input:makeTarget}",
            "options": {
                "cwd": "${fileDirname}"
            },
            "presentation": {
                "reveal": "silent",
                "panel": "shared",
                "showReuseMessage": false,
                "clear": true

              },        
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ],
    "inputs": [
        {
            "type": "pickString",
            "id": "makeTarget",
            "description": "Select a make target",
            "options": [
                {   
                    "value": "flash",
                    "label": "compile and upload code (upload)"
                },
                {   
                    "value": "compile",
                    "label": "only compile code (verify)"
                },
                {   
                    "value": "clean",
                    "label": "remove non-source files"
                },
                {   
                    "value": "complete",
                    "label": "complete re-compile without upload"
                },
                {   
                    "value": "verbose",
                    "label": "verbose upload to debug serial connection"
                },
                {   
                    "value": "env",
                    "label": "print env variables being used"
                },
                {   
                    "value": "size",
                    "label": "print code sizes"
                },
                {   
                    "value": "help",
                    "label": "print make commands"
                }
            ],
            "default": " flash"
        }
    ]
}
```

## Using Bloom and avr-gdb
[Bloom and ATtiny13A](https://bloom.oscillate.io/docs/target/attiny13a)

### .gdbinit - place in home folder

```
set history save on
set history size 10000
set history filename ~/.gdb_history

file main.elf
target remote :1442
set listsize 0
set tui compact-source on
tui focus cmd

define ll
load
l main
end

define td
tui disable
end

define te
tui enable
end
```

## bloom.yaml - place in home folder
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

  atmel_ice_13a:
    shutdown_post_debug_session: true

    tool:
      name: "atmel_ice"
 
    target:
      name: "attiny13a"
      physical_interface: "debug_wire"
      hardware_breakpoints: true
      manage_dwen_fuse_bit: true

    server:
      name: "avr_gdb_rsp"
      ip_address: "127.0.0.1"
      port: 1442

  snap_13a:
    shutdown_post_debug_session: true

    tool:
      name: "snap"
 
    target:
      name: "attiny13a"
      physical_interface: "debug_wire"
      hardware_breakpoints: true
      manage_dwen_fuse_bit: true

    server:
      name: "avr_gdb_rsp"
      ip_address: "127.0.0.1"
      port: 1442
```

### Steps

1. Copy *.gdbinit* in your home folder
2. Copy *bloom.yaml* file in your home folder
3. Open two windows in your **CLI**
1. In first window:
```bash
cd ATtiny
bloom snap_13a
# bloom will initialize then wait for gdb server
```
2. In second window (remain in this window):
```
cd ATtiny/examples/blink
avr-gdb
```

### Typical gdb commands

* Once gdb has started, hit enter to get the command window
* `ll` to load main.elf and list contents
* `c` to run
* *Ctrl-c* to stop
* `mon reset` to reset PC to *0x0000*
* `br lineno` to set a *breakpoint* on a line number
* `display variablename` to see a variable valuable every *breakpoint*
* `set variablename=n` to change a value of the *variablename*

* to use register commands below, turn off *tui* using `td`
* `mon lr` to list ALL registers
* `mon rr portb` to show contents of *PORTB*, *DDRB* and *PINB*
* `mon wr portb portb 07` to set lowest 3 bits to 1

#### bloom and using register commands

*avr-gdb* has a bug when using the *TUI* and terminal color commands, it will be fixed in version 17. Until then, use *td* to *disable the TUI* and *te* to *enable the TUI*. The commands below have had the *TUI* disabled.

#### Example: Setting Pins
```bash
(gdb) mon wr portb portb 0
Writing value 0x00 (8-bit) to "PORTB" register, at address 0x00000038, via `data` address space...
Register written
(gdb) mon lr portb
---------- "PORTB" (`portb`) peripheral registers ----------

`portb`, `pinb`, "PINB", 0x00000036, 8-bit, "Input Pins, Port B"
`portb`, `ddrb`, "DDRB", 0x00000037, 8-bit, "Data Direction Register, Port B"
`portb`, `portb`, "PORTB", 0x00000038, 8-bit, "Data Register, Port B"

(gdb) mon rr portb
Reading "PORTB" peripheral registers...

`portb`, `pinb`, "PINB", 0x00000036, 8-bit | 0x20 (32, 0b00100000)
`portb`, `ddrb`, "DDRB", 0x00000037, 8-bit | 0x07 (7, 0b00000111)
`portb`, `portb`, "PORTB", 0x00000038, 8-bit | 0x00 (0, 0b00000000)

(gdb) mon wr portb portb 07
Writing value 0x07 (8-bit) to "PORTB" register, at address 0x00000038, via `data` address space...
Register written
```

#### Example: Setting PWM frequency
```bash
(gdb) mon lr tc0
---------- "TC0" (`tc0`) peripheral registers ----------

`tc0`, `gtccr`, "GTCCR", 0x00000048, 8-bit, "General Timer Conuter Register"
`tc0`, `ocr0b`, "OCR0B", 0x00000049, 8-bit, "Timer/Counter0 Output Compare Register"
`tc0`, `tccr0a`, "TCCR0A", 0x0000004F, 8-bit, "Timer/Counter Control Register A"
`tc0`, `tcnt0`, "TCNT0", 0x00000052, 8-bit, "Timer/Counter0"
`tc0`, `tccr0b`, "TCCR0B", 0x00000053, 8-bit, "Timer/Counter Control Register B"
`tc0`, `ocr0a`, "OCR0A", 0x00000056, 8-bit, "Timer/Counter0 Output Compare Register"
`tc0`, `tifr0`, "TIFR0", 0x00000058, 8-bit, "Timer/Counter0 Interrupt Flag register"
`tc0`, `timsk0`, "TIMSK0", 0x00000059, 8-bit, "Timer/Counter0 Interrupt Mask Register"

(gdb) mon wr tc0 ocr0a 90
Writing value 0x90 (8-bit) to "OCR0A" register, at address 0x00000056, via `data` address space...
Register written
```

## avr-gdb commands

### Basic Control Commands
- ```run``` or ```r``` - Start program execution from the beginning
- ```continue``` or ```c``` - Continue execution after a breakpoint
- ```step``` or ```s``` - Execute one line of code, stepping into functions
- ```next``` or ```n``` - Execute one line of code, stepping over functions
- ```finish``` - Run until current function returns
- ```kill``` - Terminate the running program

### Breakpoint Commands
- ```break main``` or ```b main``` - Set breakpoint at function main
- ```break 25``` - Set breakpoint at line 25 of current file
- ```break file.c:30``` - Set breakpoint at line 30 of file.c
- ```info breakpoints``` or ```i b``` - List all breakpoints
- ```delete 1``` - Delete breakpoint number 1
- ```disable 2``` - Temporarily disable breakpoint 2
- ```enable 2``` - Re-enable breakpoint 2

### Variable Inspection
- ```print variable``` or ```p variable``` - Print value of variable
- ```print/x variable``` - Print in hexadecimal format
- ```print/t variable``` - Print in binary format
- ```display variable``` - Automatically show variable value at each stop
- ```undisplay 1``` - Remove automatic display number 1
- ```info locals``` - Show all local variables
- ```info registers``` - Display all register values

### Watchpoint Commands
- ```watch variable``` - Break when variable value changes
- ```watch *0x20``` - Watch memory address 0x20
- ```rwatch variable``` - Break on read access to variable
- ```awatch variable``` - Break on any access (read or write)
- ```info watchpoints``` - List all watchpoints

### Stack and Memory Commands
- ```backtrace``` or ```bt``` - Show call stack
- ```frame 2``` - Select stack frame 2
- ```up``` - Move up one stack frame
- ```down``` - Move down one stack frame
- ```x/10xb 0x60``` - Examine 10 bytes in hex starting at 0x60
- ```x/5i $pc``` - Display 5 instructions starting at program counter

### Program Flow
- ```list``` or ```l``` - Show source code around current line
- ```list 20,30``` - Show lines 20-30
- ```disassemble``` - Show assembly of current function
- ```stepi``` or ```si``` - Execute one machine instruction
- ```nexti``` or ```ni``` - Execute one instruction (skip calls)

### Miscellaneous
- ```set var variable = 5``` - Change variable value to 5
- ```info functions``` - List all functions
- ```help command``` - Get help for specific command
- ```quit``` or ```q``` - Exit gdb


