# env.make required for *make*

## 1. In your CLI
Enter the two commands below in your termainal to copy the default environment variables.
```bash
cd ATtiny
cp env.dev env.make
```

## 2. If changes need to be made, use your GUI editor or open using nano editor.

```
nano env.make

# make changes such as the USB identifier of your programmer

# CTRL-s (save) then CTRL-x (exit)
```

## Example *env.make*

```make
MCU = attiny13a
SERIAL = /dev/ttyACM0
F_CPU = 1200000UL
USB_BAUD = 250000UL
PROGRAMMER_TYPE = atmelice_isp
PROGRAMMER_ARGS = -F -V -P usb -b 115200

# MCU = attiny13a
# SERIAL = /dev/ttyACM0
# F_CPU = 1200000UL
# USB_BAUD = 250000UL
# PROGRAMMER_TYPE = snap_isp
# PROGRAMMER_ARGS = -F -V -P usb -b 115200
```
