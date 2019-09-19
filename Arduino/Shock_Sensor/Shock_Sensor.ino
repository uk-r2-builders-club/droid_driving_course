
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiUdp.h>
#include "config.h"

const byte address_bits[] =  { 13,12,14,16 };
const int lighttime = 10000;
unsigned int address;
unsigned int localPort = 8888;
char packetBuffer[64]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged\r\n";       // a string to send back

#define LED_PIN 2
#define CELEBRATION 10

MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;

WiFiUDP Udp;
ESP8266WiFiMulti WiFiMulti;

long timer = 0;

void setup() {
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

  char sensor_name[20];
  sprintf(sensor_name, "bump_sensor%02d", address);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(sensor_name);
  WiFiMulti.addAP(ssid, pass);

  int timeout = 0;
  while (WiFiMulti.run() != WL_CONNECTED) {
     delay(1000);
     Serial.print("Connecting..");
     timeout ++;
     if (timeout > 30) {
        break;
     }
  }
  if ((WiFiMulti.run() == WL_CONNECTED)) {
     Serial.println("Connected!");
  } else {
     Serial.println("TIMEOUT!");
  }
  Udp.begin(localPort);

  ArduinoOTA.setHostname(sensor_name);
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
  
  Wire.begin();
  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
}

void loop() {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    float mag = sqrt((ax * ax) + (ay * ay) + (az * az));
/*    
 *      
    Serial.print(ax); Serial.print(",");
    Serial.print(ay); Serial.print(",");
    Serial.print(az); Serial.print(",");
    Serial.print(gx); Serial.print(",");
    Serial.print(gy); Serial.print(",");
    Serial.print(gz); Serial.print(",");
    */
    Serial.println(mag);
    delay(10);
}
