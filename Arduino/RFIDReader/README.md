This code is for a cheap wemos copy using a ESP-WROOM-02, along with an 18650 holder and charger. 

https://www.aliexpress.com/item/WeMos-D1-Esp-Wroom-02-Motherboard-ESP8266-Mini-WiFi-Nodemcu-Module-18650-Battery/32821749383.html

The RFID reader used is:

https://www.aliexpress.com/item/PN532-NFC-RFID-Module-V3-Kits-Reader-Writer/32452824672.html

GND -> GND
3.3 -> VCC
D2 --> SS 
D4 --> SCK
D6 --> MOSI
D7 --> MISO

config.h needs to be created from the default file. This contains the SSID details for your wifi, along with the base API address.

