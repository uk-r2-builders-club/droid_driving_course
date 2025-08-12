#define USE_FASTLED 1
#ifdef USE_FASTLED
 #include <FastLED.h>
#else
 #include <Adafruit_NeoPixel.h>
#endif

#include <Arduino.h>
#include <Bounce2.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <HTTPClient.h>
#include "config.h"

#define USE_SERIAL Serial
#define DIGITS 5

// "WeMos" WiFi&Bluetooth Battery, 80Mhz, 921600

////////////////////////////////////////////////////////////////////////////////
#define NUMPIXELS 43 // Number of LEDs in a strip (some are actually 56, 
#define DATAPIN0 14 //digit 0 NeoPixel 60 strip far RIGHT
#define DATAPIN1 32 //digit 1 (plus lower colon dot)
#define DATAPIN2 15 //digit 2 (plus upper colon dot)
#define DATAPIN3 33 //digit 3 (plus decimal dot)
#define DATAPIN4 27 //digit 4 far LEFT
const int pins[DIGITS] = { 14, 32, 15, 33, 27 };


#ifdef USE_FASTLED
 CRGB strip[DIGITS][NUMPIXELS];
#else

Adafruit_NeoPixel strip[] = { //here is the variable for the multiple strips
  Adafruit_NeoPixel(NUMPIXELS, pins[0], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, pins[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, pins[2], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, pins[3], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, pins[4], NEO_GRB + NEO_KHZ800)
};
#endif
const int bright = 200; //adjust brightness for all pixels 0-255 range,
// 32 being pretty dim, 255 being full brightness

#define DEBOUNCE 10
#define FLASHINTERVAL 500

#define RESETPIN 26
#define STARTPIN 25
#define PAUSEPIN 34
#define RESUMEPIN 19
#define STOPPIN 18

#define CELEBRATION 10

const int segments[10][8] = {
  { 1,1,1,1,1,1,0,1 },
  { 0,0,1,1,0,0,0,1 },
  { 0,1,1,0,1,1,1,1 },
  { 0,1,1,1,1,0,1,1 },
  { 1,0,1,1,0,0,1,1 },
  { 1,1,0,1,1,0,1,1 },
  { 1,1,0,1,1,1,1,1 },
  { 0,1,1,1,0,0,0,1 },
  { 1,1,1,1,1,1,1,1 },
  { 1,1,1,1,1,0,1,1 },
};

const int colours[11][3] = {
  { 0, 0, 0 },
  { 255,0,0 },
  { 0,255,0 },
  { 0,0,255 },
  { 160,160,160 },
  { 255,255,0 },
  { 255,0,255 },
  { 255,255,0 },
  { 0,255,255 },
  { 175,0,255 },
  { 0, 0, 255 }   
};

// segs are 0-5, 6-11, 12-17, 18-23, 24-29, 30-35, 36-41, 42
const int segs[8][2] {
  { 0, 5 },
  { 6, 11 },
  { 12, 17 },
  { 18, 23 },
  { 24, 29 },
  { 30, 35 },
  { 36, 41 },
  { 42, 42 }
};

#define NUM_BUTTONS 5
const uint8_t BUTTON_PINS[NUM_BUTTONS] = { RESETPIN, STARTPIN, PAUSEPIN, RESUMEPIN, STOPPIN };

Bounce * buttons = new Bounce[NUM_BUTTONS];
String pressed = "";
char display_time[DIGITS + 1] = "00000";
char last_display[DIGITS + 1] = "00000";
unsigned int localPort = 8888;
char packetBuffer[64]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged\r\n";       // a string to send back

unsigned long course_time;
unsigned long last_flash;
unsigned long start_time;
int course_state = 0;
int clock_colour = 1;
int clock_flash = 0;
int clock_flash_state = LOW;

WiFiUDP Udp;
WiFiMulti WiFiMulti;
HTTPClient http;
////////////////////////////////////////////////////////////////////////////////

void setup() {
////////////////////////////////////////////////////////////////////////////////
  byte i;
  USE_SERIAL.begin(115200);
  
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
    buttons[i].interval(25);              // interval in ms
  }
  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, pass);

  // allow reuse (if server supports it)

  delay(500); //pause a moment to let capacitors on board settle

  #ifdef USE_FASTLED
  FastLED.addLeds<NEOPIXEL, DATAPIN0>(strip[0], NUMPIXELS);
  FastLED.addLeds<NEOPIXEL, DATAPIN1>(strip[1], NUMPIXELS);
  FastLED.addLeds<NEOPIXEL, DATAPIN2>(strip[2], NUMPIXELS);
  FastLED.addLeds<NEOPIXEL, DATAPIN3>(strip[3], NUMPIXELS);
  FastLED.addLeds<NEOPIXEL, DATAPIN4>(strip[4], NUMPIXELS);

  //flash an zero
  for(int t=0;t<DIGITS;t++){
    digitWrite(t,0,0); //clear it
    FastLED.show();
    digitWrite(t,0,1); //display it
    FastLED.show();
  }   
  
  #else
  //NeoPixel array setup
  for(int s=0;s<DIGITS;s++){
    strip[s].begin(); // Initialize pins for output
    strip[s].setBrightness(bright);
    strip[s].show();
    delay(200);
  }    
  //flash an zero
  for(int t=0;t<DIGITS;t++){
    digitWrite(t,0,0); //clear it
    strip[t].show();
    digitWrite(t,0,1); //display it
    strip[t].show();
  }
  #endif
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
     rainbowLights();
  } else {
     Serial.println("TIMEOUT!");
  }
    http.setReuse(true);
  Udp.begin(localPort);

  ArduinoOTA.setHostname("timer");
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
////////////////////////////////////////////////////////////////////////////////
}


////////////////////////////////////////////////////////////////////////////////
void loop() {   
  for (int i=0; i < NUM_BUTTONS; i++) {
    buttons[i].update();
  }

  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    Udp.read(packetBuffer, 64);
    Serial.println("UDP Broadcast received. Contents:");
    Serial.println(packetBuffer);
    if (strcmp(packetBuffer, "rainbow") == 0) {
      for (int c = 0; c < CELEBRATION; c++) {
        rainbowLights();
      }
    } else if (strcmp(packetBuffer, "reset") == 0) {
      sprintf(display_time, "00000");
      course_time = 0;
      course_state = 0;
      clock_colour = 1;
      clock_flash = 0;
      for (int d = 0; d<DIGITS; d++) {
        digitWrite(d, display_time[d] - '0', clock_colour);
      }
    } else if (strcmp(packetBuffer, "tilt") == 0) {
      Serial.println("TILT");
    }
  }
  
  if (buttons[0].fell() ) {
    pressed = "RESET";
    sprintf(display_time, "00000");
    course_time = 0;
    course_state = 0;
    clock_colour = 1;
    clock_flash = 0;
    for (int d = 0; d<DIGITS; d++) {
      digitWrite(d, display_time[d] - '0', clock_colour);
    }
  }

  if (buttons[1].fell() && course_state == 0) {
    pressed = "START";
    course_time = 0;
    start_time = millis();
    course_state = 1;
    clock_colour = 2;
    clock_flash = 0;
  }

  if (buttons[2].fell() && course_state == 1) {
    pressed = "MIDDLE_WAIT";
    course_time = millis() - start_time;
    course_state = 2;
    USE_SERIAL.print("First half time: ");
    USE_SERIAL.println(course_time);
    clock_colour = 3;
    clock_flash = 1;
  }

  if (buttons[3].fell() && course_state == 2 ) {
    pressed = "MIDDLE_START";
    course_state = 3;
    start_time = millis();
    clock_colour = 2;
    clock_flash = 0;
  }

  if (buttons[4].fell() && course_state == 3 ) {
    pressed = "FINISH";
    course_time = course_time + (millis() - start_time);
    USE_SERIAL.print("Final time: ");
    USE_SERIAL.println(course_time);    
    course_state = 4;
    clock_colour = 5;
    clock_flash = 1;
  }
  
  if (pressed != "") {
    send_state(pressed, course_time);
    pressed = "";
  }

  if (course_state == 1) {
    int minutes = ((millis() - start_time)/60000);
    int seconds = ((millis() - start_time - (minutes * 60000))/1000);
    int tenths = ((millis() - start_time)%1000)/100;
    sprintf(display_time, "%02d%02d%01d", minutes, seconds, tenths);
  }
  if (course_state == 3) {
    int minutes = (((millis() - start_time)+course_time)/60000);
    int seconds = ((((millis() - start_time)+course_time) - (minutes * 60000))/1000);
    int tenths = (((millis() - start_time)+course_time)%1000)/100;
    sprintf(display_time, "%02d%02d%01d", minutes, seconds, tenths);
  }

  if (course_state == 1 || course_state == 3) { // Clock only runs in these two states
    for (int i=0; i<DIGITS; i++) {
        digitWrite(i, display_time[i] - '0', clock_colour);
    }
  }

  if (clock_flash == 1) {
    unsigned long currentMillis = millis();
    if (currentMillis - last_flash >= FLASHINTERVAL) {
      last_flash = currentMillis;
      if (clock_flash_state == LOW) {
        for (int i=0; i<DIGITS; i++) {
          digitWrite(i, display_time[i] - '0', 0);
          clock_flash_state = HIGH;
        }
      } else {
        for (int i=0; i<DIGITS; i++) {
          digitWrite(i, display_time[i] - '0', clock_colour);
          clock_flash_state = LOW;
        }
      }
    }
  }
  FastLED.show();
}
//END void loop()
////////////////////////////////////////////////////////////////////////////////

void send_state(String state, unsigned long course_time) {
  String api_call = String((char*)api) + "run/" + state + "/" + String(course_time, DEC);
  if ((WiFiMulti.run() == WL_CONNECTED)) {
      USE_SERIAL.print("API Call: ");
      USE_SERIAL.println(api_call);
      http.begin(api_call);
      int httpCode = http.GET();
      http.end();
  } else {
      USE_SERIAL.println("Failed to send");
  }
}

void rainbowLights() {
  USE_SERIAL.println("Rainbow pattern");
  for (int i = 0; i < 10; i++) {
    for (int d = 0; d<DIGITS; d++) {
      digitWrite(d, display_time[d] - '0', i);
      FastLED.show();
    }
    delay(100);
  }
  for (int d = 0; d<DIGITS; d++) {
    digitWrite(d, display_time[d] - '0', clock_colour);
  }
}

////////////////////////////////////////////////////////////////////////////////
void digitWrite(int digit, int val, int col){
  //use this to light up a digit
  //digit is which digit panel one (right to left, 0 indexed)
  //val is the character value to set on the digit
  //col is the predefined color to use, R,G,B or W
  //example: 
  //        digitWrite(0, 4, 2); 
  //would set the first digit
  //on the right to a "4" in green.

  /*
  // Letters are the standard segment naming, as seen from the front,
  // numbers are based upon the wiring sequence
  
            A 2     
       ----------
      |          |
      |          |
  F 1 |          | B 3
      |          |
      |     G 7  |
       ----------
      |          |
      |          |
  E 6 |          | C 4
      |          |
      |     D 5  |
       ----------    dp 8
  
  */
  //these are the digit panel character value definitions, 
  //if color argument is a 0, the segment is off
  for (int i = 1; i < 9; i++) {
    if (segments[val][i] == 1) {
      segLight(digit, i, col); 
    } else {
      segLight(digit, i, 0); 
    }
  }
}
//END void digitWrite()
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
void segLight(char digit, int seg, int col){ 
  //use this to light up a segment
  //digit picks which neopixel strip
  //seg calls a segment
  //col is color
  int color[3] = { 
    colours[col][0],
    colours[col][1],
    colours[col][2]
  };

  for(int i=segs[seg][0]; i < segs[seg][1] + 1; i++) {
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif    
  }

}
//END void segLight()
////////////////////////////////////////////////////////////////////////////////
