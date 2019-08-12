/**************************************************************************/
/*! 
    @file     RFIDReader
    @author   Darren Poulson
	  @license  BSD (see license.txt)

    This sketch will wait for any ISO14443A card or tag programmed
    up for the UK R2 Builders club

    Results will be displayed on the attached LCD screen. Valid card IDs will
    be sent to the R2 Builders driving course.

    Use TTGO LoRa32-OLED V1, 80MHz, 921600, None

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
//#include <SoftSPI.h>
// #include <Adafruit_PN532.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>                                  // Core graphics library
#include <Adafruit_ST7735.h>                               // Hardware-specific library

#include "config.h"

#define SPEAKER_PIN 25
#define TONE_PIN_CHANNEL 0

// Use this line for a breakout with a software SPI connection (recommended):
//Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
//SoftSPI mySPI(27,26,32);
PN532_SPI pn532spi(SPI, 21);
NfcAdapter nfc = NfcAdapter(pn532spi);
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
//  wifi.hostname("rfidreader");
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

  if (nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    Serial.println(tag.getTagType());
    Serial.print("UID: ");Serial.println(tag.getUidString());

    if (tag.hasNdefMessage()) // every tag won't have a message
    {
      lastRead=millis();
      NdefMessage message = tag.getNdefMessage();
      
      // R2 Builders tags should have 4 records:
      //   Text: UK R2 Builders Club
      //   URL: http://astromech.info
      //   Text: Details
      //   URL: Link to PLI info
      //
      // If there aren't 4 records, reject with message.
      
      if (message.getRecordCount() != 4) {
        Serial.println("Not enough records");
        tone(400, 100);
        tft.fillRect(0,35,200,200,ST7735_BLACK);
        tft.setCursor(4, 44);
        tft.print("Invalid Tag");
        card_displayed = 1;
      } else {
        NdefRecord record = message.getRecord(0);
        Serial.print("  TNF: ");Serial.println(record.getTnf());
        Serial.print("  Type: ");Serial.println(record.getType()); // will be "" for TNF_EMPTY
        int payloadLength = record.getPayloadLength();
        byte payload[payloadLength];
        record.getPayload(payload);
        Serial.print("  Payload (HEX): ");
        PrintHexChar(payload, payloadLength);
        

        // Force the data into a String (might work depending on the content)
        // Real code should use smarter processing
        String payloadAsString = "";
        for (int c = 3; c < payloadLength; c++) {
          payloadAsString += (char)payload[c];
        }
        Serial.print("  Payload (as String): ");
        Serial.println(payloadAsString);
        if (record.getType() != "T") {// || payloadAsString != "enUK R2 Builders Club" ){
          Serial.print("First record is not a Text block");
          tone(400, 100);
          tft.fillRect(0,35,200,200,ST7735_BLACK);
          tft.setCursor(4, 44);
          tft.print("Not a builders card");
          card_displayed = 1;
        } else {
          // Ok, so its got four records, and first one is a text block containing "UK R2 Builders Club"
          // Lets read in the information in block 3 to register on the course.
          NdefRecord record = message.getRecord(2);
          Serial.print("  TNF: ");Serial.println(record.getTnf());
          Serial.print("  Type: ");Serial.println(record.getType()); // will be "" for TNF_EMPTY
          int payloadLength = record.getPayloadLength();
          byte payload[payloadLength];
          record.getPayload(payload);
          Serial.print("  Payload (HEX): ");
          PrintHexChar(payload, payloadLength);
          // Force the data into a String (might work depending on the content)
          // Real code should use smarter processing
          char payloadAsString[payloadLength-3];
          for (int c = 3; c < payloadLength; c++) {
            payloadAsString[c-3] = (char)payload[c];
          }
          payloadAsString[payloadLength-3] = NULL;

          // payloadAsString now contains:
          // Droid name
          // Droid uid
          // Member name
          // Member uid
          Serial.print("Card data: ");
          Serial.println(payloadAsString);
          char *droid_name = strtok(payloadAsString, "\n");
          char *droid_uid = strtok (NULL, "\n");
          char *member_name = strtok (NULL, "\n");
          char *member_uid = strtok (NULL, "\n");

          //sscanf(payloadAsString,"%s\n%s\n%s\n%s", &droid_name, &droid_uid, &member_name, &member_uid);
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
      }
    }
  }
  delay(2000);
  
}
