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

  int timeout = 0;
  while (WiFiMulti.run() != WL_CONNECTED) {
     delay(1000);
     Serial.print("Connecting..");
     pulseLights();
     timeout ++;
     if (timeout > 30) {
        break;
     }
  }
  if ((WiFiMulti.run() == WL_CONNECTED)) {
     Serial.println("Connected!");
     startupLights();
     matrix.fillScreen(matrix.Color(0, 0, 0));
     matrix.show();
  } else {
     Serial.println("TIMEOUT!");
     flashLights(10, 100);
     matrix.fillScreen(matrix.Color(0, 0, 0));
     matrix.show();
  }
}

void flashLights(int count, int wait) {
  int i;
  for(i=0; i<count; i++) {
     matrix.fillScreen(matrix.Color(255,0,0));
     matrix.show();
     delay(wait);
     matrix.fillScreen(matrix.Color(0,0,0));
     matrix.show();
     delay(wait);
  }
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

void startupLights() {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<9; i++) {
      matrix.fillScreen(Wheel((i+j) & 255));
    }
    matrix.show();
    delay(5);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void pulseLights() {
  uint16_t i, j;
  for(i=0; i<50; i++) {
    matrix.fillScreen(matrix.Color(i*5,0,0));
    matrix.show();
    delay(5);
  }
  for(j=50; j>0; j--) {
    matrix.fillScreen(matrix.Color(j*5,0,0));
    matrix.show();
    delay(5);
  }
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
    failLights();
    String api_call = String((char*)api) + "gate/" + address + "/FAIL";
    Serial.print("API Call: ");
    Serial.println(api_call);

    http.begin(client, api_call);
    int httpCode = http.GET();
    http.end();
  }
  delay(10);

}
