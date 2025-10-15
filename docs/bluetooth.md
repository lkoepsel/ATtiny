# Raspberry Pi Bluetooth HC06 Notes

## Raspberry Pi Software Required

**install Serial Port Profile (SPP) support**  
```bash
sudo apt update  
sudo apt install pulseaudio-bluetooth  
sudo reboot  
```

## Pairing with the Raspberry Pi

```bash  
## to attempt to connect w/ Bookworm s/w
sudo bluetoothctl  
## scan for devices, look for MAC Address (Use AT+ADDR to get HCO6 MAC)  
scan on  
## use MAC address to pair  
pair 00:23:09:01:67:7D  
## when prompted enter PIN (1234)  
Request PIN code  
[agent] Enter PIN code: 1234
## Pairing works with bluetoothctl, connecting does not 
exit 
## this will bind to a /dev address, so you can use tio  
sudo rfcomm bind 0 00:23:09:01:67:7D  
## its best to use line mode  (once connect, HC06 will indicate paired)
tio -b 9600 -input-mode line /dev/rfcomm0  
```

### If above commands worked, then didn't
You will need to remove the device and start over
```bash
sudo bluetoothctl
remove 00:23:09:01:67:7D
```

## AT Commands

[Arduino Forum Advice](https://forum.arduino.cc/t/getting-hc-06-back-into-at-command-mode/509739/20)

In CoolTerm, it is best to be in Line Mode, CR+LF seems to make it easier to read, not required

• AT - check connection and responds with OK    
• AT+ADDR - gets the address of the device *ex:* AT+ADDR  
• AT+NAME - replaces name, *ex:* AT+SENSOR3     
• AT+UART - checks the baud, adding ":" allows you to change *ex:* AT+UART:9600,0,0 <BAUD, STOP BIT, PARITY>  
• AT+PSWD - checks PIN, adding ":" allows you to change *ex:* AT+PSWD:"1234"  
• AT+ORGL - restores factory defaults: BAUD = 9600,0,0
PIN = 1234


## Sources 

### BEST LINK FOR HC06 Documentation  
[The Complete Guide To The HC-06 – Martyn Currey](https://www.martyncurrey.com/the-complete-guide-to-the-hc-06/)

### Arduino softSerial Library  
[SoftwareSerial Library | Arduino Documentation](https://docs.arduino.cc/learn/built-in-libraries/software-serial/)

### CoolTerm seems to work best  
https://freeware.the-meiers.org/

## CODE THAT WORKS

### Simple LED program, 1 for On and 0 for off

#### Setup
* Connect Arduino Pins 2/3 to TX/RX on HC06
* Serial connection to Uno, use CoolTerm

```c
// PIN 2 is RX and PIN 3 is TX (remember to switch)  
#include <SoftwareSerial.h>  
SoftwareSerial softSerial(2, 3); // RX, TX

char Incoming_value=0;

void setup() {  
  softSerial.begin(9600);  
  Serial.begin(9600);  
  pinMode(13, OUTPUT);  
  digitalWrite(13, HIGH);  
  delay(1000);  
  digitalWrite(13, LOW);  
  Serial.print("Ready");  
}

void loop() {  
  if(softSerial.available()\>0)  
  {  
    Incoming_value = softSerial.read();  
    Serial.print(Incoming_value);  
    Serial.print("\\n");

    if(Incoming_value == '1')  
    {  
      digitalWrite(13, HIGH);  
      Serial.print("HIGH");  
    }  
    else if(Incoming_value == '0')  
    {  
      digitalWrite(13, LOW);  
      Serial.print("LOW");  
    }  
  }  
}
```

### CODE THAT COMMUNICATES WITH HC06 from Link above Martyn Curry  
This code allows you to interact with the module using AT commands. The HC06 won't respond to AT commands if paired. See table for AT commands to use.
```c
/*
 *  sketch: SerialPassThrough_SoftwareSerial_Basic
 *  martyncurrey.com
 *   
 *  Use software serial to talk to serial/UART connected device
 *  What ever is entered in the serial monitor is sent to the connected device
 *  Anything received from the connected device is copied to the serial monitor
 * 
 *  Pins
 *  BT VCC to Arduino 5V out. 
 *  BT GND to GND
 *  Arduino D2 (Arduino RX) goes to module TX
 *  Arduino D3 (Arduino TX) goes to module RX. May need a voltage divider.
 * 
 *  Assumes a 5V Arduino is being used
 *  If the connected device is 3.3v add a voltage divider (5v to 3.3v) between Arduino TX and device RX
 *  Arduino RX to device TX does not need a voltage divider. The Arduino will see 3.3v as high
 * 
 */

#include <SoftwareSerial.h>
SoftwareSerial softSerial(2, 3); // RX, TX
 
char c=' ';

 
void setup() 
{
    Serial.begin(9600,SERIAL_8N1 );
   Serial.print("Sketch:   ");   Serial.println(__FILE__);
    Serial.print("Uploaded: ");   Serial.println(__DATE__);
    softSerial.begin(9600);
    Serial.println("softSerial started at 9600");
    Serial.println("Ready");
}

 
void loop()
{
    // Read from the Serial Monitor and send to the UART module
    if (Serial.available())
    {
        c = Serial.read();
         softSerial.write(c);
    }

    // Read from the UART module and send to the Serial Monitor
    if (softSerial.available())
    {
        c = softSerial.read();
        Serial.write(c); 
    }
    
} // void loop()
```
 
## Can a Macboook Pro connect to a HC06  
### Short Answer  
No, not reliably. The HC-06 uses Bluetooth Classic (SPP profile), which macOS doesn't natively support for direct connections.

### Technical Details

**HC-06 Specifications:**  
* Bluetooth 2.0 Classic  
* Serial Port Profile (SPP) only  
* Designed for microcontroller UART communication

**macOS Bluetooth Stack:**  
* Supports Bluetooth Low Energy (BLE) natively  
* Supports standard Bluetooth Classic profiles (A2DP, HID, etc.)  
* Does NOT support SPP (Serial Port Profile) without third-party drivers

## Workarounds

### 1. USB-to-Bluetooth Adapter  
Use a USB Bluetooth dongle with SPP support and third-party drivers

### 2. Microcontroller Bridge  
Connect HC-06 to an Arduino/ESP32, then use USB serial connection to Mac

### 3. Replace Module  
Switch to a BLE module compatible with macOS:  
* HM-10 (BLE 4.0)  
* ESP32 (supports both Classic and BLE)  
* nRF52 series

### 4. Third-Party Software  
Some apps claim SPP support on macOS, but results are inconsistent and often require deprecated APIs

## Recommended Solution  
For M2 MacBook Pro compatibility, use a **BLE module** (HM-10 or ESP32) instead of HC-06. This provides native macOS support without additional drivers.

## Complete Steps to Reconnect

1. **Restart the Bluetooth controller:**
   ```bash
   bluetoothctl
   power off
   power on
   ```

2. **Enable the agent and make your Pi discoverable:**[^7][^8]
   ```bash
   agent on
   default-agent
   discoverable on
   pairable on
   ```

3. **Start scanning:**
   ```bash
   scan on
   ```
   Wait for the HC-06 to appear in the scan results[^5]

4. **Once the HC-06 appears, pair and connect:**[^3]
   ```bash
   pair [HC-06_MAC_ADDRESS]
   trust [HC-06_MAC_ADDRESS]
   connect [HC-06_MAC_ADDRESS]
   ```

## Additional Sources of Information

[^1]: [HC06 Bluetooth Module Guide with Arduino Interfacing](https://microcontrollerslab.com/hc06-bluetooth-module-pinout-arduino-interfacing-examples/) (31%)  
[^2]: [HC-06 Bluetooth Module Hookup Guide - Edwin Robotics](https://learn.edwinrobotics.com/hc-06-bluetooth-module-hookup-guide/) (30%)    
[^3]: [usb - Share files between 2 computers via bluetooth from... - Ask Ubuntu](https://askubuntu.com/questions/838697/share-files-between-2-computers-via-bluetooth-from-terminal) (14%)  
[^4]: [Getting HC-06 back into AT-command mode? - Arduino Forum](https://forum.arduino.cc/t/getting-hc-06-back-into-at-command-mode/509739) (11%)   
[^5]: [Raspberry Pi 3 Unable to Connect to Arduino with HC-06](https://forums.raspberrypi.com/viewtopic.php?t=163712) (7%)  
[^6]: [Connect to a Bluetooth Device via the Terminal | Baeldung on Linux](https://www.baeldung.com/linux/bluetooth-via-terminal) (7%)  
[^7]: [Checking the Battery Level of a Connected... | Baeldung on Linux](https://www.baeldung.com/linux/check-bluetooth-device-battery-level) (< 1%)    
[^8]: [Connect to a headless Raspberry Pi using Bluetooth - Medium](https://medium.com/@tomw3115/connect-to-a-headless-raspberry-pi-using-bluetooth-0e61c05e1b68) (< 1%)    
