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
 *  ######################
 *  Hardware 
 *  
 *  This runs on an Arduino Mega (Needed for the memory), with SPI connections on:
 *  
 *  MISO - 50
 *  MOSI - 51
 *  SCK  - 52
 *  SS   - 53
 *  
 *  Along with power and ground
 *  
 *  
 */

#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_SPI pn532spi(SPI, 53);
NfcAdapter nfc = NfcAdapter(pn532spi);

char droid_name[16] = "R2D2";
char droid_uid[4] = "0";
char member_name[32] = "Kenny Baker";
char member_uid[4] = "0";
char id[60] = "";

char id_url[100] = "";
char main_info[56] = "";

char title[22] = "UK R2D2 Builders Club";                           // Main text file
char site_url[32] = "http://astromech.info/";                       // Site URL
char mot_url[40] = "https://mot.astromech.info/id.php?id=";         // Base URL for ID link

char line[128] = "";
int l=0;

void setup() {
      Serial.begin(115200);
      Serial.println("NDEF Writer");
      nfc.begin();
}

void loop() {
    Serial.println("Enter data");
    Serial.println(line);
    id[0] = (char)0;
    line[0] = (char)0;
    droid_name[0] = (char)0;
    droid_uid[0] = (char)0;
    member_name[0] = (char)0;
    member_uid[0] = (char)0;


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
    
    Serial.println("\nPlace a formatted Mifare Classic or Ultralight NFC tag on the reader.");
    bool writetag=false;
    while(!writetag) {
    if (nfc.tagPresent()) {
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
        } else {
          Serial.println("Write failed.");
          writetag=true;
        }
    }
    delay(1000);
    }

    dataReady = false;
}
