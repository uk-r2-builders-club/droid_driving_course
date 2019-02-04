/**************************************************************************/
/*! 
    @file     readMifare.pde
    @author   Adafruit Industries
	@license  BSD (see license.txt)

    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    - Authenticate block 4 (the first block of Sector 1) using
      the default KEYA of 0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we can then read any of the
      4 blocks in that sector (though only block 4 is read here)
	 
    If the card has a 7-byte UID it is probably a Mifare
    Ultralight card, and the 4 byte pages can be read directly.
    Page 4 is read by default since this is the first 'general-
    purpose' page on the tags.


This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
This library works with the Adafruit NFC breakout 
  ----> https://www.adafruit.com/products/364
 
Check out the links above for our tutorials and wiring diagrams 
These chips use SPI or I2C to communicate.

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

*/
/**************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "config.h"

String droid_name;
String droid_uid;
String member_name;
String member_uid;

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (12)
#define PN532_SS   (4)
#define PN532_MISO (13)

#define BUZZER (5)
#define LED (0)

ADC_MODE(ADC_VCC);

// Use this line for a breakout with a software SPI connection (recommended):
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

uint8_t last_uid[] = { 0, 0, 0, 0, 0, 0, 0 };

void setup(void) {
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
}


void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    Serial.print("  Last UID: ");
    nfc.PrintHex(last_uid, uidLength);
    Serial.println("");
    
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
	  
      // Now we need to try to authenticate it for read/write access
      // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      Serial.println("Trying to authenticate block 4 with default KEYA value");
      //uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      uint8_t keya[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
	  
	  // Start with block 4 (the first block of sector 1) since sector 0
	  // contains the manufacturer data and it's probably better just
	  // to leave it alone unless you know what you're doing
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
	  
      if (success)
      {
        Serial.println("Sector 1 (Blocks 4..7) has been authenticated");
        uint8_t data1[16];
        uint8_t data2[16];
        uint8_t data3[16];
        uint8_t data4[16];
        uint8_t data[64];

        // Try to read the contents of block 4
        success = nfc.mifareclassic_ReadDataBlock(4, data1);
		    success = nfc.mifareclassic_ReadDataBlock(5, data2);
        success = nfc.mifareclassic_ReadDataBlock(6, data3);
        success = nfc.mifareclassic_ReadDataBlock(7, data4);

        if (success)
        {
          // Data seems to have been read ... spit it out
          Serial.println("Reading Block 4-7:");
          nfc.PrintHexChar(data1, 16);
          nfc.PrintHexChar(data2, 16);
          nfc.PrintHexChar(data3, 16);
          nfc.PrintHexChar(data4, 16);
          Serial.println("");
          memcpy(data, data1, 16);
          memcpy(data+16, data2, 16);
          memcpy(data+32, data3, 16);
          memcpy(data+48, data4, 16);
          //nfc.PrintHexChar(data, 64);

          droid_name = "";
          droid_uid = "";
          member_name = "";
          member_uid = "";
          int i = 11;
          while(data[i] != 0x0A)
          {
            droid_name += (char)data[i];
            i++;
          }
          i++;
          while(data[i] != 0x0A)
          {
            droid_uid += (char)data[i];
            i++;
          }
          i++;
          while(data[i] != 0x0A)
          {
            member_name += (char)data[i];
            i++;
          }
          i++;
          while(data[i] != 0xFE)
          {
            member_uid += (char)data[i];
            i++;
          }
          Serial.print("Droid Name: ");
          Serial.println(droid_name);
          Serial.print("Droid UID: ");
          Serial.println(droid_uid);
          Serial.print("Member Name: ");
          Serial.println(member_name);
          Serial.print("Member UID: ");
          Serial.println(member_uid);

          String memberCall = String((char*)api) + "member/" + member_uid;
          String droidCall = String((char*)api) + "droid/" + droid_uid;
      
          Serial.println("New card presented.... send API call");
          Serial.println("Register driver");
          Serial.println(memberCall);
          HTTPClient http;
          http.begin(memberCall);
          int httpCode = http.GET();
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
              String payload = http.getString();
              Serial.print("Response OK: ");
              Serial.println(payload);
            }
          } else {
            Serial.println("Error");
          }
          http.end();
          Serial.println("Register droid");
          Serial.println(droidCall);
          http.begin(droidCall);
          httpCode = http.GET();
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
              String payload = http.getString();
              Serial.print("Response OK: ");
              Serial.println(payload);
            }
          } else {
            Serial.println("Error");
          }
           http.end();

          // Lets notify that a card was read ok
          tone(BUZZER, 1000);
          digitalWrite(LED, HIGH);
          delay(500);
          noTone(BUZZER);
          digitalWrite(LED, LOW);
          // Wait a bit before reading the card again
          delay(2000);
          Serial.print("VCC Voltage: ");
          Serial.println(ESP.getVcc());
        }
        else
        {
          Serial.println("Ooops ... unable to read the requested block.  Try another key?");
        }
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
      }
    }
    
    delay(1000);
  }
}
