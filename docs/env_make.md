# env.make required for *make*

## 1. In your CLI
Enter the two commands below to open the nano editor, in the *AVR_C* folder
```bash
cd ATtiny
nano env.make
```

## 2. In the nano editor
Copy and paste the text below. Be sure to go to step 3!

### env.make
```make
# ATtiny13A, be sure to set LIBRARY = no_lib as many AVR_C library functions are not compatible
MCU = attiny13a
SERIAL = /dev/ttyACM0
F_CPU = 1200000UL
USB_BAUD = 250000UL
SOFT_RESET = 0
LIBDIR = $(DEPTH)ATtiny13A
LIBRARY = no_lib
PROGRAMMER_TYPE = atmelice_isp
PROGRAMMER_ARGS = -F -V -P usb -b 115200
TOOLCHAIN =
OS =
SOFT_BAUD = 28800UL
```

## 3. Save and close *nano*

*CTRL-s (save) then CTRL-x (exit)*
