#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>

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

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(3, 3, LED_PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };

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

  Serial.print("Address: ");
  Serial.println(address);
  
  pinMode(INPUT_PASS_PIN, INPUT);
  pinMode(INPUT_FAIL_PIN, INPUT);

  matrix.begin();
  matrix.setBrightness(70);
  matrix.fillScreen(matrix.Color(0, 0, 0));
  matrix.show();
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname("bump_sensor");
  WiFiMulti.addAP(ssid, pass);

  while (WiFiMulti.run() != WL_CONNECTED) {
     delay(1000);
     Serial.print("Connecting..");
  }
  Serial.println("Connected!");


}

void passLights() {
  matrix.fillScreen(matrix.Color(0,255,0));
  matrix.show();
  delay(10000);
  matrix.fillScreen(matrix.Color(0, 0, 0));
  matrix.show();
}

void failLights() {
  matrix.fillScreen(matrix.Color(255,0,0));
  matrix.show();
  delay(10000);
  matrix.fillScreen(matrix.Color(0, 0, 0));
  matrix.show();
}

WiFiClient client;
HTTPClient http;

void loop() {
  int pass = digitalRead(INPUT_PASS_PIN);
  int fail = digitalRead(INPUT_FAIL_PIN);
  if (pass == LOW) {
     Serial.println("Pass hit");
     passLights();
  } 
  if (fail == LOW) {
    Serial.println("Fail hit");
    String api_call = String((char*)api) + "gate/" + address + "/FAIL";
    Serial.print("API Call: ");
    Serial.println(api_call);

    http.begin(client, api_call);
    int httpCode = http.GET();
    http.end();
    failLights();
  }
  delay(10);

}
