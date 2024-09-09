# Protogen Flex Glove
The protogen flex glove is a peripheral for a protogen LED head that uses finger movement to control face gesture.  
The flex gloves use ESP32 based boards and Bluetooth Low Energy (BLE) for communication.  
An ESP32 is used as the bluetooth server and main microcontroller for the protogen head, and two Seed Studio XIAO ESP32C3 for the gloves.  
ESP-WROOM32 and Arduino ESP32 nano will be officially supported. Other ESP32 based boards should still work, but will need more tweaking with the code. 




## Components used:
*this list does not include the protogen head, just the ESP32 controlling it*
- [XIAO EPS32C2](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html), 2pc 
- [ESP-WROOM32](https://www.amazon.com/s?k=esp32+Wroom+32&i=electronics&crid=2EIN54VSP3B0&sprefix=esp32+wroom+%2Celectronics%2C166&ref=nb_sb_noss_2) or [Arduino ESP32 nano](https://store.arduino.cc/products/nano-esp32), 1pc 
- [2.2 inch flex sensor](https://www.adafruit.com/product/1070), 6pc 
- [Swich for battery](https://www.sparkfun.com/products/9609), 2pc
- [Lithuim ion polymer battery](https://www.adafruit.com/product/1578), 2pc
- [PCB prototype board](https://www.amazon.com/Prototype-Soldering-Universal-Printed-Electronic/dp/B079DN31SW?source=ps-sl-shoppingads-lpcontext&ref_=fplfs&psc=1&smid=A1CDIFZ5ZDIZVB) (using 4x6cm board), 2pc
  
Total Cost: ~$110

(flex sensors are wildly expensive)


Both flex gloves use the same script, they are able to select between left or right mode by a 10kΩ resistor on pin 8. When connected to ground, the client will be in left mode, and connected to power it will be in right mode. The script only checks this value on startup, so the client mode cannot be changed during runtime.

## Pin Connection
### Xiao 
- A0 -> thumb/middle sensor
- A1 -> index sensor
- A2 -> middle/thumb sensor
- D8 -> 10kΩ -> power (right) / ground (left)
- [Battery pads](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#battery-usage) -> slide switch -> battery

The circuit for each glove shold be the same. since glove clients are able to differentiate between left and right, it is able to change wich sensors correspond to thumb or middle finger.

Flex sensors use a pulldown resistor (between ground and input), we reccomend using 1kΩ . With a 1kΩ resistor, analog input ranges from 0 to ~150, default threashold is 75 but each sensor can have its own threshold value.

The XIAOs include a small antenna extension for bluetooth and wifi, but it is still able to function at a shorter distance without it, so the antenna is not required.


Circuit diagram

![](.repo/images/schematic.png)


## Addressing Clients
The addresses are used to check which client connects and disconnects. Changing the address of the Xiao is kinda weird, so instead we reccomend using its already initialized address. To find the address, simply connect the client to the server, and the address will be printed out to the serial monitor, copy the series of hex numbers and enter them into either leftADDR[] or rightADDR[].

## Current Progress

First glove version.

![](.repo/images/glove.jpg)

(Accidentally burned a hole in the glove while soldering)


Paws in action

![IMG_6820 (2) (2)](https://github.com/user-attachments/assets/acb3166c-c815-4afc-8eeb-c204bfc12c73)

(gonna get a better demo video sometime later)



## Development Galery

First prototype, using potentiometers to simulate flex sensors.

![](.repo/images/prototype1.jpg)


Prototpe using flex sensors

![](.repo/images/prototype2.jpg)

Server with OLED display

![](.repo/images/serverOn.jpg)
