# Protogen Flex Glove
The protogen flex glove is a peripheral for a protogen LED head that uses finger movement to control face gesture.  
The flex gloves use ESP32 based boards and Bluetooth Low Energy for communication.  
An ESP32 is used as the bluetooth server and main microcontroller for the protogen head, and two Seed Studio XIAO ESP32C3 for the gloves.  
ESP32-WROOM and Arduino ESP32 nano will be officially supported. Other ESP32 based boards should still work, but will need more tweaking with the code.  



## Components used:
*this list does not include the protogen head, just the ESP32 controlling it*
- [XIAO EPS32C2](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html), 2pc 
- [ESP32-WROOM](https://www.amazon.com/s?k=esp32+Wroom+32&i=electronics&crid=2EIN54VSP3B0&sprefix=esp32+wroom+%2Celectronics%2C166&ref=nb_sb_noss_2) or [Arduino ESP32 nano](https://store.arduino.cc/products/nano-esp32), 1pc 
- [2.2 inch flex sensor](https://www.adafruit.com/product/1070), 6pc 
- [Lilon/Lipoly QTpy charger BFF](https://www.adafruit.com/product/5397), 2pc (optional)
- [Lilon/Lipoly fuel gauge](https://www.adafruit.com/product/4712), 2pc (optional)
- [Swich for battery](https://www.sparkfun.com/products/9609), 2pc (if not using QTpy charger)
- [Lithuim ion polymer battery](https://www.adafruit.com/product/1578), 2pc
- [Mini PCB prototype board](https://www.amazon.com/ElectroCookie-Solderable-Breadboard-Electronics-Gold-Plated/dp/B081MSKJJX), 2pc



For a lower profile board, the QTpy charger is not necessary, as the XIAO has [battery pad on the underside](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#battery-usage). Instead a battery can be directly soldered to the pads with a switch between.

Both flex gloves use the same script, but are able to differentiate if they are they are the left or right glove by a 10kΩ resistor on pin 8. When connected to ground, the client will be in left mode, and connected to power it will be in right mode. The script only checks this value on startup, so the client mode cannot be changed during runtime.

## Pin Connection
### Xiao 
- A0 -> thumb sensor
- A1 -> index sensor
- A2 -> middle senor
- D4 -> SDA Lilon/Lipoly fuel gauge (optional)
- D5 -> SCL Lilon/Lipoly fuel guage (optional)
- D8 -> 10kΩ -> power (right) / ground (left)
  
  all flex sensors use a pullup resistor between their respective input pins and power
