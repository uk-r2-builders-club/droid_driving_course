#include "DFRobotDFPlayerMini.h"

#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/10, /*tx =*/4);
#define FPSerial softSerial

#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>

#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);


const byte address_bits[] =  { 13,12,14,16 };
const int lighttime = 10000;
unsigned int address;
unsigned int localPort = 8888;
char packetBuffer[64]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged\r\n";       // a string to send back

#define INPUT_PASS_PIN D4
#define INPUT_FAIL_PIN D3
#define LED_PIN D1
#define CELEBRATION 10

WiFiUDP Udp;
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

  FPSerial.begin(9600);
  myDFPlayer.begin(FPSerial);
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.volume(20);  //Set volume value. From 0 to 30
  myDFPlayer.play(1);  //Play the first mp3
  
  pinMode(INPUT_PASS_PIN, INPUT);
  pinMode(INPUT_FAIL_PIN, INPUT);

  matrix.begin();
  matrix.setBrightness(70);
  matrix.fillScreen(matrix.Color(0, 0, 0));
  matrix.show();

  char sensor_name[20];
  sprintf(sensor_name, "bump_sensor%02d", address);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(sensor_name);
  WiFiMulti.addAP(ssid, pass);

  int timeout = 0;
  while (WiFiMulti.run() != WL_CONNECTED) {
     Serial.print("(");
     Serial.print(timeout);
     Serial.println(")Connecting..");
     pulseLights();
     timeout ++;
     if (timeout > 10) {
        break;
     }
  }
  if ((WiFiMulti.run() == WL_CONNECTED)) {
     Serial.println("Connected!");
     myDFPlayer.play(2);
     startupLights();
     matrix.fillScreen(matrix.Color(0, 0, 0));
     matrix.show();
  } else {
     Serial.println("TIMEOUT!");
     myDFPlayer.play(3);
     flashLights(10, 100);
     matrix.fillScreen(matrix.Color(0, 0, 0));
     matrix.show();
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
}

void failLights() {
  matrix.fillScreen(matrix.Color(255,0,0));
  matrix.show();
}

void offLights() {
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
  ArduinoOTA.handle();
  int pass = digitalRead(INPUT_PASS_PIN);
  int fail = digitalRead(INPUT_FAIL_PIN);
  if (pass == LOW) {
     Serial.println("Pass hit");
     myDFPlayer.play(2);
     passLights();
     delay(lighttime);
     offLights();
  } 

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    Udp.read(packetBuffer, 64);
    Serial.println("UDP Broadcast received. Contents:");
    Serial.println(packetBuffer);
    if (strcmp(packetBuffer, "rainbow") == 0) {
      for (int c = 0; c < CELEBRATION; c++) {
        startupLights();
        matrix.fillScreen(matrix.Color(0, 0, 0));
        matrix.show();
      }
    }
  }

  if (packetSize == 8 && strncmp(packetBuffer, "volume", 6) == 0) {
    // Check if the 7th and 8th characters are digits
    if (isdigit(packetBuffer[6]) && isdigit(packetBuffer[7])) {
      // The message is in the format 'volumeXX'
      // Extract the two-digit number
      char numStr[3];
      numStr[0] = packetBuffer[6];
      numStr[1] = packetBuffer[7];
      numStr[2] = '\0'; // Null-terminate the string
      
      int volume = atoi(numStr); // Convert the string to an integer
      
      // Now you can use the 'volume' variable for your programming
      // For example:
      // set_volume(volume);
      
      Serial.print("Received volume command: ");
      Serial.println(volume);
      myDFPlayer.volume(volume);
    }
  }

  
  if (fail == LOW) {
     Serial.println("Fail hit");
     myDFPlayer.play(3);
     failLights();
     String api_call = String((char*)api) + "gate/" + address + "/FAIL";
     Serial.print("API Call: ");
     Serial.println(api_call);
     http.begin(client, api_call);
     int httpCode = http.GET();
     http.end();
     delay(lighttime);
     offLights();
  }
  delay(10);

}
