#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>

#include <Adafruit_NeoPixel.h>

#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const byte address_bits[] =  { 13,12,14,16 };
unsigned int address;

#define INPUT_PASS_PIN 4
#define INPUT_FAIL_PIN 0
#define LED_PIN 2

ESP8266WiFiMulti WiFiMulti;

void setup() {
  // put your setup code here, to run once
  Serial.begin(115200);
  Serial.println("Startup");
  for (byte x = 0 ; x < 4 ; x ++ ) {
    Serial.print("Configuring Pin: ");
    Serial.println(address_bits[x]);
    pinMode(address_bits[x], INPUT);
  }

  Serial.println("Read in address");
  for (byte x = 0 ; x < 4 ; x ++ ) {
    byte value = !digitalRead(address_bits[x]);
    Serial.print("Bit: ");
    Serial.print(x);
    Serial.print(" Value: ");
    Serial.println(value);
    address = address + (value << x);
  }
  // Make the address 1-16 rather than 0-15
  address = address + 1;

  Serial.print("Address: ");
  Serial.println(address);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("bump_sensor");
  WiFiMulti.addAP(ssid, pass);

  while (WiFiMulti.run() != WL_CONNECTED) {
     delay(1000);
     Serial.print("Connecting..");
  }
  Serial.println("Connected!");

  pinMode(INPUT_PASS_PIN, INPUT);
  pinMode(INPUT_FAIL_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
}

void passLights() {
  
}

void failLights() {
  
}

void loop() {
  int pass = digitalRead(INPUT_PASS_PIN);
  int fail = digitalRead(INPUT_FAIL_PIN);
  if (pass == LOW) {
     Serial.println("Pass hit");
     passLights();
     delay(10000);
  } 
  if (fail == LOW) {
    Serial.println("Fail hit");
    String api_call = String((char*)api) + "gate/" + address + "/FAIL";
    Serial.print("API Call: ");
    Serial.println(api_call);
    WiFiClient client;
    HTTPClient http;
    http.begin(client, api_call);
    int httpCode = http.GET();
    http.end();
    failLights();
    delay(10000);
  }

}
