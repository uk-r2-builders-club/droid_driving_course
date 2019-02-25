/**************************************************************************/
/*! 
    @file     RFIDReader
    @author   Darren Poulson
	  @license  BSD (see license.txt)

    This sketch will wait for any ISO14443A card or tag programmed
    up for the UK R2 Builders club

    Results will be displayed on the attached LCD screen. Valid card IDs will
    be sent to the R2 Builders driving course

This is based on the example sketch for the Adafruit PN532 NFC/RFID 
breakout boards
  ----> https://www.adafruit.com/products/364

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

*/
/**************************************************************************/
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>                                  // Core graphics library
#include <Adafruit_ST7735.h>                               // Hardware-specific library

#include "config.h"

String droid_name;
String droid_uid;
String member_name;
String member_uid;

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (32)
#define PN532_MOSI (27)
#define PN532_SS   (21)
#define PN532_MISO (26)

#define SPEAKER_PIN 25
#define TONE_PIN_CHANNEL 0

// Use this line for a breakout with a software SPI connection (recommended):
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
Adafruit_ST7735 tft = Adafruit_ST7735(16, 17, 23, 5, 9);
WiFiMulti wifi;

uint32_t _count;
bool speaker_on;
unsigned long lastRead;
bool card_displayed;
int tft_height, tft_width;

#define r2_logo_width 48
#define r2_logo_height 60
static unsigned char r2_logo_bits[] = {
   0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x01, 0x00,
   0x00, 0x00, 0xf8, 0xff, 0x07, 0x00, 0x00, 0x00, 0x3e, 0xe0, 0x0f, 0x00,
   0x00, 0x00, 0x06, 0x00, 0x1f, 0x00, 0x00, 0x80, 0x03, 0x00, 0x1e, 0x00,
   0x00, 0x80, 0x01, 0x00, 0x38, 0x00, 0x00, 0xc0, 0x98, 0x00, 0x38, 0x00,
   0x00, 0x60, 0xc8, 0x07, 0x70, 0x00, 0x00, 0x60, 0xc4, 0x3f, 0x70, 0x00,
   0x00, 0x30, 0xc4, 0xff, 0x70, 0x00, 0x00, 0x30, 0xf4, 0xff, 0x73, 0x00,
   0x00, 0x38, 0x00, 0xfe, 0x6f, 0x00, 0x00, 0x18, 0x00, 0xe0, 0x7f, 0x00,
   0x00, 0x0e, 0x80, 0x03, 0x7f, 0x00, 0x00, 0x0f, 0xc0, 0x1f, 0x78, 0x00,
   0x00, 0x07, 0xe0, 0x3f, 0xe0, 0x00, 0x80, 0x07, 0x30, 0x7c, 0x00, 0x01,
   0x80, 0x03, 0x18, 0xf8, 0x00, 0x0e, 0xc0, 0x03, 0x18, 0xe0, 0x00, 0x00,
   0xe0, 0x03, 0x18, 0xe0, 0x01, 0x00, 0xe0, 0x01, 0x1c, 0xc0, 0x01, 0x00,
   0xe0, 0x01, 0x1c, 0xc0, 0x00, 0x00, 0xf0, 0x00, 0x1c, 0x00, 0x00, 0x00,
   0x70, 0x00, 0x18, 0x00, 0x00, 0x00, 0x78, 0x00, 0x18, 0x00, 0x00, 0x00,
   0x38, 0x00, 0x38, 0x80, 0x00, 0x00, 0x30, 0x00, 0x38, 0xc0, 0x00, 0x00,
   0x18, 0x00, 0x70, 0xc0, 0x00, 0x00, 0x18, 0x00, 0x70, 0xe0, 0x00, 0x00,
   0x0c, 0x00, 0xf0, 0xc0, 0x00, 0x00, 0x0c, 0x00, 0xe0, 0xe0, 0x00, 0x00,
   0x06, 0x00, 0xe0, 0xe0, 0x00, 0x00, 0xc6, 0x01, 0xc0, 0xc1, 0x01, 0x00,
   0xe0, 0x0f, 0xc0, 0xc1, 0x01, 0x00, 0xe0, 0x3f, 0x80, 0xc3, 0x01, 0x00,
   0x00, 0xff, 0x81, 0xc1, 0x01, 0x00, 0x00, 0xff, 0x07, 0x81, 0x03, 0x00,
   0x00, 0xe7, 0x1f, 0x80, 0x03, 0x00, 0x00, 0x8e, 0xff, 0x80, 0x07, 0x00,
   0x00, 0x0e, 0xfc, 0x01, 0x07, 0x00, 0x00, 0x5c, 0xf0, 0x00, 0x0f, 0x00,
   0x00, 0xfc, 0x07, 0x00, 0x1e, 0x00, 0x00, 0xfe, 0x0f, 0x00, 0x1e, 0x00,
   0x80, 0xff, 0x1f, 0x00, 0x7c, 0x00, 0x80, 0x03, 0x0e, 0x00, 0x78, 0x00,
   0xe0, 0x01, 0x08, 0x00, 0xf0, 0x01, 0x70, 0x00, 0x00, 0x00, 0xe0, 0x01,
   0x38, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x1c, 0x00, 0x00, 0x00, 0x80, 0x0f,
   0x0c, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x06, 0x00, 0x00, 0x00, 0x00, 0x1e,
   0x06, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x06, 0x00, 0x00, 0x00, 0x00, 0x38,
   0xff, 0x07, 0x00, 0x00, 0x00, 0x38, 0xfe, 0x07, 0x00, 0x00, 0x00, 0x38,
   0xfc, 0x03, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x80, 0xff, 0x0f,
   0x00, 0x00, 0x00, 0x80, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00 };
   
void tone(uint8_t frequency, uint8_t duration) {
   ledcWriteTone(TONE_PIN_CHANNEL, frequency);
   _count = millis() + duration;
   speaker_on = 1;  
}

void drawScreen() {
  tft.fillScreen(ST7735_BLACK); 
  tft.setCursor(4,0);
  tft.print("R2 Builders");
  tft.setCursor(4,12);
  tft.print("  Driving Course");
  tft.drawFastHLine(0,24,200,ST7735_BLUE);
  tft.drawXBitmap((tft_height/2)-(r2_logo_width/2),(tft_height/2)-(r2_logo_height/2), r2_logo_bits, r2_logo_width, r2_logo_height, 0x333333);
  tft.setCursor(35,44);
  tft.print("Please Swipe");
  tft.setCursor(55,56);
  tft.print("Tag");
}

void setup(void) {  
  Serial.begin(115200);
  // Init Screen 
  tft.initR(INITR_18GREENTAB);                             // 1.44 v2.1
  tft.fillScreen(ST7735_BLACK);                            // CLEAR
  tft.setTextColor(0x5FCC);                                // GREEN
  tft.setRotation(1);
  tft_height = tft.height();
  tft_width = tft.width();
  Serial.print("Width of screen: ");
  Serial.println(tft_width);
  Serial.print("Height of screen: ");
  Serial.println(tft_height);
  tft.drawXBitmap((tft_height/2)-(r2_logo_width/2),(tft_height/2)-(r2_logo_height/2), r2_logo_bits, r2_logo_width, r2_logo_height, 0xdd);

  // Setup Speaker
  ledcSetup(TONE_PIN_CHANNEL, 0, 13);
  ledcAttachPin(SPEAKER_PIN, TONE_PIN_CHANNEL);  

  // Connect to Wifi   
  tft.setCursor(4,0);
  tft.print("Connecting to Wifi");

  Serial.print("Connecting to ");
  Serial.println(ssid);
  //wiFi.mode(WIFI_STA);
  wifi.addAP(ssid, pass);
  while (wifi.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("   Connected");
  tft.setCursor(4,12);
  tft.print("Connected...");



  // Initialise NFC Reader
  tft.setCursor(4,24);
  tft.print("Initialising Reader");
  tft.setCursor(4,36);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    tft.print("FAILED!");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC); 
  // configure board to read RFID tags
  nfc.SAMConfig(); 
  nfc.setPassiveActivationRetries(0x10);
  tft.print("Done");
  delay(500);
  Serial.println("Waiting for an ISO14443A Card ...");
  
  // Setup done
  drawScreen();
  tone(1000,100);
  
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Mute Speaker
  if(speaker_on) {
     if(millis() > _count) {
         speaker_on = 0;
         Serial.println("Muting Speaker");
         ledcWriteTone(TONE_PIN_CHANNEL, 0);
         digitalWrite(SPEAKER_PIN, 0);
     }
  }

  // Clear screen of names
  if (millis() > lastRead + 2000 && card_displayed) {
     card_displayed = 0;
     drawScreen();
  }    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success && lastRead + 2000 < millis()) {
    lastRead = millis();
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
	  
      // Now we need to try to authenticate it for read/write access
      // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      Serial.println("Trying to authenticate block 4 with default KEYA value");
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
          tft.fillRect(0,35,200,200,ST7735_BLACK);
          tft.setCursor(4, 44);
          tft.print(member_name);
          tft.setCursor(4, 56);
          tft.print(droid_name);

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
            Serial.print("Error: ");
            Serial.println(httpCode);
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
            Serial.print("Error: ");
            Serial.println(httpCode);
          }
           http.end();

          // Lets notify that a card was read ok
          // Wait a bit before reading the card again
          tone(1000, 100);
          card_displayed = 1;
        }
        else
        {
          Serial.println("Ooops ... unable to read the requested block.  Try another key?");
          tone(600, 100);
          tft.fillRect(0,35,200,200,ST7735_BLACK);
          tft.setCursor(4, 44);
          tft.print("Failed to read tag");
          card_displayed = 1;
        }
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
        tone(400, 100);
        tft.fillRect(0,35,200,200,ST7735_BLACK);
        tft.setCursor(4, 44);
        tft.print("Invalid Tag");
        card_displayed = 1;
      }
    }   
  }
}
