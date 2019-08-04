#include <FastLED.h>
#define NUM_LEDS 14


#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "config.h"

CRGBArray<NUM_LEDS> leds;
#define CELEBRATION_LENGTH 10000
#define DEFAULT_DELAY 50


unsigned int localPort = 8888;
char packetBuffer[64]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged\r\n";       // a string to send back
int celebration = 0;
unsigned long celebration_start = 0;
int celebration_delay = DEFAULT_DELAY;

WiFiUDP Udp;
ESP8266WiFiMulti WiFiMulti;

void setup() { 

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, pass);
  
  FastLED.addLeds<NEOPIXEL,2>(leds, NUM_LEDS); 
  for(int i = 0; i < NUM_LEDS; i++) {   
      leds.fadeToBlackBy(150);
      //leds[i] = CHSV(160,255,255);
  }

  int timeout = 0;
  while (WiFiMulti.run() != WL_CONNECTED) {
     delay(1000);
     Serial.print("Connecting..");
     timeout ++;
     if (timeout > 120) {
        break;
     }
  }
  if ((WiFiMulti.run() == WL_CONNECTED)) {
     Serial.println("Connected!");
     //rainbowLights();
  } else {
     Serial.println("TIMEOUT!");
  }
  Udp.begin(localPort);

  ArduinoOTA.setHostname("strobe");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
//END void setup()
  
}

void loop(){ 

  if (millis() - celebration_start > CELEBRATION_LENGTH) {
         celebration = 0;
         celebration_delay = DEFAULT_DELAY;
  }
    
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    Udp.read(packetBuffer, 64);
    Serial.println("UDP Broadcast received. Contents:");
    Serial.println(packetBuffer);
    if (strcmp(packetBuffer, "rainbow") == 0) {
         celebration = 1;
         celebration_start = millis();
    } else if (strcmp(packetBuffer, "reset") == 0) {
         celebration = 0;
         celebration_start = millis();
    } else if (strcmp(packetBuffer, "fast") == 0) {
         celebration_delay = 20;
         celebration_start = millis();
    } else if (strcmp(packetBuffer, "slow") == 0) {
         celebration_delay = 150;
         celebration_start = millis();
    }
  }

  switch (celebration) {
    case 0:  
      for(int i = 0; i < NUM_LEDS; i++) {   
        leds.fadeToBlackBy(150);
        leds[i] = CHSV(160,255,255);
        FastLED.delay(celebration_delay);
      }
      break;
    case 1:
      for(int i = 0; i < NUM_LEDS; i++) {   
        //int colour;
        //colour = i*(255/i+1);
        leds.fadeToBlackBy(150);
        leds[i] = CHSV(i*25,255,255);
        FastLED.delay(celebration_delay);
      }
      break;
  }
}
