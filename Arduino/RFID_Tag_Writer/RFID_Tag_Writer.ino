/* 
 *  ######################
 *  Overview
 *  
 *  Takes in a CSV line of the format via serial:
 *  
 *       droid name, droid ID, member name, member ID, ID code
 *  
 *  You will the place a tag on the reader/writer to write the correct details
 *  
 *    
 *  
 */

#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Adafruit_GFX.h>                                  // Core graphics library
#include <Adafruit_ST7735.h>                               // Hardware-specific library

#include "config.h"

#define SPEAKER_PIN 25
#define TONE_PIN_CHANNEL 0

PN532_SPI pn532spi(SPI, 0);
NfcAdapter nfc = NfcAdapter(pn532spi);

#define LED_BUILTIN 22
Adafruit_ST7735 tft = Adafruit_ST7735(16, 17, 23, 5, 9); // CS,A0,SDA,SCK,RESET

char droid_name[16] = "R2D2";
char droid_uid[4] = "0";
char member_name[32] = "Kenny Baker";
char member_uid[4] = "0";
char id[60] = "";

char id_url[100] = "";
char main_info[56] = "";

char line[128] = "";
int l=0;

void setup() {
      Serial.begin(115200);
      Serial.println("NDEF Writer");
      pinMode(27,OUTPUT);
      digitalWrite(27,HIGH);
      tft.initR(INITR_GREENTAB);                             // 1.44 v2.1
      tft.fillScreen(ST7735_BLACK);                            // CLEAR
      tft.setTextColor(0x5FCC);                                // GREEN
      tft.setRotation(1);                                      // 
      pinMode(LED_BUILTIN, OUTPUT);
      tft.setCursor(4,3);
      tft.print("R2 Badge Writer");
      tft.setCursor(4,12);
      tft.print("Select badge to write");
      nfc.begin();
      digitalWrite(LED_BUILTIN, LOW);


}

void loop() {

    id[0] = (char)0;
    line[0] = (char)0;
    droid_name[0] = (char)0;
    droid_uid[0] = (char)0;
    member_name[0] = (char)0;
    member_uid[0] = (char)0;

    Serial.println("Enter data");
    //Serial.println(line);

    id_url[0] = (char)0;
    main_info[0] = (char)0;
    delay(1000);
    bool dataReady = false;
    l=0;
    while(!dataReady) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        Serial.print(inByte);

        if (inByte == 13 || inByte == 10) {
          dataReady = true;
          Serial.println();
          Serial.print("End of line detected: ");
          Serial.print(inByte, OCT);
          Serial.println();
          line[l] = char(0);
        } else {
          line[l] = inByte;
        }
        //Serial.println((char)line[l]);
        l++;
      }
    }
    Serial.println("Data Ready");
    Serial.println("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
  
    char * strtokIndx;

    // Data format = Droid Name, Droid ID, Member Name, Member ID, PLI Status ID
    // test data - test,1,test,1,1
    
    strtokIndx = strtok(line,",");
    strcpy(droid_name, strtokIndx);

    strtokIndx = strtok(NULL,",");
    strcpy(droid_uid, strtokIndx);
    
    strtokIndx = strtok(NULL,",");
    strcpy(member_name, strtokIndx);

    strtokIndx = strtok(NULL,",");
    strcpy(member_uid, strtokIndx);

    strtokIndx = strtok(NULL,",");
    strcpy(id, strtokIndx);
    
    strcat(main_info, droid_name);
    strcat(main_info, "\n");
    strcat(main_info, droid_uid);
    strcat(main_info, "\n");
    strcat(main_info, member_name);
    strcat(main_info, "\n");
    strcat(main_info, member_uid);
        
    Serial.print("Droid Name:     ");
    Serial.println(droid_name);

    Serial.print("Droid ID:       ");
    Serial.println(droid_uid);

    Serial.print("Member Name:    ");
    Serial.println(member_name);

    Serial.print("Member ID:      ");
    Serial.println(member_uid);

    Serial.print("Badge ID:       ");
    Serial.println(id);
    
    Serial.print("Title:          ");
    Serial.println(title);
    Serial.println("Details:      ");
    Serial.println(main_info);
    
    Serial.print("MOT URL:        ");
    Serial.println(mot_url);
    strcat(id_url, mot_url);
    strcat(id_url, id);
    Serial.print("PLI Status URL: ");
    Serial.println(id_url);

    delay(1000);

    tft.fillScreen(ST7735_BLACK);                            // CLEAR                               // 
    tft.setCursor(4,3);
    tft.print(member_name);
    tft.setCursor(4,12);
    tft.print(droid_name);
    tft.setCursor(4,25);
    tft.print("Ready to write");
    tft.setCursor(4,34);
    tft.print("Place tag now");
    
    
    Serial.println("\nPlace a formatted Mifare Classic or Ultralight NFC tag on the reader.");
    bool writetag=false;
    while(!writetag) {
    if (nfc.tagPresent()) {

        Serial.println("Formatting...");
        bool format = nfc.format();
        if (!format) {
            Serial.println("Failed to format.");
        }
        Serial.println("Trying to write.....");
        NdefMessage message = NdefMessage();
        message.addTextRecord(title);
        message.addUriRecord(site_url);
        message.addTextRecord(main_info);
        message.addUriRecord(id_url);

        bool success = nfc.write(message);
        if (success) {
          Serial.println("Success. Try reading this tag with your phone.");        
          writetag=true;
          tft.fillScreen(ST7735_BLACK);                            // CLEAR                               // 
          tft.setCursor(4,3);
          tft.print("Success!");
        } else {
          Serial.println("Write failed.");
          writetag=true;
          tft.fillScreen(ST7735_BLACK);                            // CLEAR                               // 
          tft.setCursor(4,3);
          tft.print("FAIL!!!!");         
        }
    }
    delay(1000);


    }
    tft.setCursor(4,20);
    tft.print("Next..."); 
    dataReady = false;
}
