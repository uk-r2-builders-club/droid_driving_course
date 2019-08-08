#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);

String droid_name;
String droid_uid;
String member_name;
String member_uid;
String title = "UK R2D2 Builders Club";
String id;
bool dataReady;

int l=0;

void setup() {
      Serial.begin(115200);
      Serial.println("NDEF Writer");
      nfc.begin();
}

void loop() {
    Serial.println("Enter data");
    delay(100);
    char line[128];
    char inByte;
    memset(line, 0, sizeof(line));
    l=0;
    while(!dataReady) {
      if (Serial.available() > 0) {
        inByte = Serial.read();
        //Serial.println(inByte);

        if (inByte == 13 || inByte == 10) {
          dataReady = true;
          Serial.print("End of line detected: ");
          Serial.print(inByte, OCT);
          Serial.println();
        }

        line[l] = inByte;
        
        //Serial.println((char)line[l]);
        l++;
      }
    }
    Serial.println("Data Ready");
    line[l] = NULL;

    droid_name = "";
    droid_uid = "";
    member_name = "";
    member_uid = "";
    id="";
    int i=0;
    while(line[i] != 44)
      {
        droid_name += (char)line[i];
        i++;
      }
    Serial.print("Droid Name: ");
    Serial.println(droid_name);
    i++;
    while(line[i] != 44)
      {
        droid_uid += (char)line[i];
        i++;
      }
    Serial.print("Droid ID: ");
    Serial.println(droid_uid);
    i++;
    while(line[i] != 44)
      {
        member_name += (char)line[i];
        i++;
      }
    Serial.print("Member Name: ");
    Serial.println(member_name);
    i++;  
    while(line[i] != 44)
      {
        member_uid += (char)line[i];
        i++;
      }
    Serial.print("Member ID: ");
    Serial.println(member_uid);
    i++;
    for (int x=i; x < i+60; x++)
    //while(line[i] != NULL || line[i] != 10 || line[i] != 13 || line[i] != 32)
      {
        id += (char)line[x];
        //Serial.println(id);
        //i++;
      }
    Serial.print("Badge ID: ");
    Serial.println(id);
    Serial.print("Title: ");
    Serial.println(title);
    Serial.println("\nPlace a formatted Mifare Classic or Ultralight NFC tag on the reader.");
    bool writetag=false;
    while(!writetag) {
    if (nfc.tagPresent()) {
        Serial.println("Trying to write.....");
        NdefMessage message = NdefMessage();
        message.addTextRecord(title);
        message.addUriRecord("http://astromech.info");
        message.addTextRecord(droid_name + "\n" + droid_uid + "\n" + member_name + "\n" + member_uid + "\n");
        message.addUriRecord("https://mot.r2djp.co.uk/id.php?id=" + id);

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
