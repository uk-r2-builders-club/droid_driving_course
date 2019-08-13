
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

MPU6050 mpu6050(Wire);
ESP8266WiFiMulti wifi;

long timer = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  wifi.addAP(ssid, pass);
  while (wifi.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("   Connected");
}

void loop() {
  mpu6050.update();
    
  Serial.print(mpu6050.getAccX());
  Serial.print(",");Serial.print(mpu6050.getAccY());
  Serial.print(",");Serial.println(mpu6050.getAccZ());

}
