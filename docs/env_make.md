# env.make required for *make*

## 1. In your CLI
Enter the two commands below to open the nano editor, in the *AVR_C* folder
```bash
cd ATtiny
cp env.dev env.make
```

## 2. Edit to make required changes, such as USB identifier

```
nano env.make

# CTRL-s (save) then CTRL-x (exit)
```

## env.make

```make
MCU = attiny13a
SERIAL = /dev/ttyACM0
F_CPU = 1200000UL
USB_BAUD = 250000UL
PROGRAMMER_TYPE = atmelice_isp
PROGRAMMER_ARGS = -F -V -P usb -b 115200
```
