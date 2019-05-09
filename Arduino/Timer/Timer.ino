#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include <WiFiClient.h>
#include "config.h"

#define USE_SERIAL Serial

// "WeMos" WiFi&Bluetooth Battery, 80Mhz, 921600

////////////////////////////////////////////////////////////////////////////////
#define NUMPIXELS 43 // Number of LEDs in a strip (some are actually 56, 
//some 57 due to extra colon/decimal points)
#define DATAPIN0 27 //digit 0 NeoPixel 60 strip far RIGHT
#define DATAPIN1 33 //digit 1 (plus lower colon dot)
#define DATAPIN2 15 //digit 2 (plus upper colon dot)
#define DATAPIN3 32 //digit 3 (plus decimal dot)
#define DATAPIN4 15 //digit 4 far LEFT

Adafruit_NeoPixel strip[] = { //here is the variable for the multiple strips
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN0, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN1, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN2, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN3, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN4, NEO_GRB + NEO_KHZ800)
};
const int bright = 200; //adjust brightness for all pixels 0-255 range,
// 32 being pretty dim, 255 being full brightness

#define RESETPIN 26
#define STARTPIN 25
#define PAUSEPIN 34
#define RESUMEPIN 39
#define STOPPIN 36

WiFiMulti wifiMulti;
WiFiClient client;
HTTPClient http;

volatile int clockstate = 0; // 0 = stopped, 1 = running

////////////////////////////////////////////////////////////////////////////////

void setup() {
////////////////////////////////////////////////////////////////////////////////

  USE_SERIAL.begin(115200);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();
  for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
  }
  wifiMulti.addAP(ssid, pass);

  pinMode(RESETPIN, INPUT_PULLUP);
  pinMode(STARTPIN, INPUT_PULLUP);
  pinMode(PAUSEPIN, INPUT_PULLUP);
  pinMode(RESUMEPIN, INPUT_PULLUP);
  pinMode(STOPPIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(STARTPIN), start_clock, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAUSEPIN), pause_clock, FALLING);
  attachInterrupt(digitalPinToInterrupt(RESUMEPIN), resume_clock, FALLING);
  attachInterrupt(digitalPinToInterrupt(STOPPIN), stop_clock, FALLING);
  attachInterrupt(digitalPinToInterrupt(RESETPIN), reset_clock, FALLING);
    
  delay(500); //pause a moment to let capacitors on board settle
  //NeoPixel array setup
  for(int s=0;s<5;s++){
    strip[s].begin(); // Initialize pins for output
    strip[s].setBrightness(bright);
    strip[s].show();
    delay(200);
  }
  
  //flash an eight
  for(int t=0;t<5;t++){
    digitWrite(t,8,0); //clear it
    strip[t].show();
    digitWrite(t,8,2); //display it
    strip[t].show();
  }
  delay(1500);
//END void setup()
////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
void loop() { 
  //this uses digitWrite() command to run through all numbers with one second 
  //timing on each of the five digit panels in the display simultaneously
  for(int i=0;i<10;i++){
    for(int j=0;j<5;j++){
      digitWrite(j, i, (i+1));
      strip[j].show();
    }
    delay(1000); //one second intervals
    for(int j=0;j<5;j++){ 
      digitWrite(j, i, 0); //clears the digit between refreshes
      strip[j].show();
    }

  }
}
//END void loop()
////////////////////////////////////////////////////////////////////////////////


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
  if (val==0){
    //segments F,A,B,C,D,E,G, dp
    segLight(digit,1,col);
    segLight(digit,2,col);
    segLight(digit,3,col);
    segLight(digit,4,col);
    segLight(digit,5,col);
    segLight(digit,6,col);
    segLight(digit,7,0);
    segLight(digit,8,col);
  }
  if (val==1){
    segLight(digit,1,0);
    segLight(digit,2,0);
    segLight(digit,3,col);
    segLight(digit,4,col);
    segLight(digit,5,0);
    segLight(digit,6,0);
    segLight(digit,7,0);
    segLight(digit,8,col);
  }
  if (val==2){
    segLight(digit,1,0);
    segLight(digit,2,col);
    segLight(digit,3,col);
    segLight(digit,4,0);
    segLight(digit,5,col);
    segLight(digit,6,col);
    segLight(digit,7,col);
    segLight(digit,8,col);
  }
  if (val==3){
    segLight(digit,1,0);
    segLight(digit,2,col);
    segLight(digit,3,col);
    segLight(digit,4,col);
    segLight(digit,5,col);
    segLight(digit,6,0);
    segLight(digit,7,col);
    segLight(digit,8,col);
  }
  if (val==4){
    segLight(digit,1,col);
    segLight(digit,2,0);
    segLight(digit,3,col);
    segLight(digit,4,col);
    segLight(digit,5,0);
    segLight(digit,6,0);
    segLight(digit,7,col);
    segLight(digit,8,col);
  }
  if (val==5){
    segLight(digit,1,col);
    segLight(digit,2,col);
    segLight(digit,3,0);
    segLight(digit,4,col);
    segLight(digit,5,col);
    segLight(digit,6,0);
    segLight(digit,7,col);
    segLight(digit,8,col);
  }
  if (val==6){
    segLight(digit,1,col);
    segLight(digit,2,col);
    segLight(digit,3,0);
    segLight(digit,4,col);
    segLight(digit,5,col);
    segLight(digit,6,col);
    segLight(digit,7,col);
    segLight(digit,8,col);
  }          
  if (val==7){
    segLight(digit,1,0);
    segLight(digit,2,col);
    segLight(digit,3,col);
    segLight(digit,4,col);
    segLight(digit,5,0);
    segLight(digit,6,0);
    segLight(digit,7,0);
    segLight(digit,8,col);
  }
  if (val==8){
    segLight(digit,1,col);
    segLight(digit,2,col);
    segLight(digit,3,col);
    segLight(digit,4,col);
    segLight(digit,5,col);
    segLight(digit,6,col);
    segLight(digit,7,col);
    segLight(digit,8,col);
  }
  if (val==9){
    segLight(digit,1,col);
    segLight(digit,2,col);
    segLight(digit,3,col);
    segLight(digit,4,col);
    segLight(digit,5,col);
    segLight(digit,6,0);
    segLight(digit,7,col);
    segLight(digit,8,col);
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
  int color[3];

  //color sets
    if (col==0){ //off
      color[0]={0};
      color[1]={0};
      color[2]={0};
    }
    if (col==1){ //red
      color[0]={255};
      color[1]={0};
      color[2]={0};
    }
    if (col==2){ //green
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }
    if (col==3){ //blue
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }
    if (col==4){ //white -- careful with this one, 3x power consumption
      //if 255 is used
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }

      if (col==5){ //yellow
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }
        if (col==6){ //
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }
        if (col==7){ //
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }
        if (col==8){ //
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }
        if (col==9){ //
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }
        if (col==10){ // 
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }

  //sets are 0-5, 6-11, 12-17, 18-23, 24-29, 30-35, 36-41, 42
  //seg F
  if(seg==1){
    //light first 6
    for(int i=0; i<6; i++){
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
    }  
  }
  //seg A
  if(seg==2){
      //light second 8
      for(int i=6; i<12; i++){
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
      } 
  }
  //seg B
  if(seg==3){
      for(int i=12; i<18; i++){
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
      }   
  }
  //seg C
  if(seg==4){
      for(int i=18; i<24; i++){
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
      }   
  }
  //seg D
  if(seg==5){
      for(int i=24; i<30; i++){
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
      }   
  }
  //seg E
  if(seg==6){
      for(int i=30; i<36; i++){
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
      }   
  }
  //seg G
  if(seg==7){
      for(int i=36; i<42; i++){
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
      }   
  }
  //seg dp
  if(seg==8){
      for(int i=42; i<43; i++){
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
      }   
  }
}
//END void segLight()
////////////////////////////////////////////////////////////////////////////////

void send_state(String state) {
  String api_call = String((char*)api) + "run/" + state;
  Serial.print("API Call: ");
  Serial.println(api_call);
  http.begin(client, api_call);
  int httpCode = http.GET();
  http.end();
}

void start_clock() {
  clockstate = 1;
  send_state("START");
}

void pause_clock() {
  clockstate = 0;
  send_state("MIDDLE_WAIT");
}

void resume_clock() {
  clockstate = 1;
  send_state("MIDDLE_START");
}

void stop_clock() {
  clockstate = 0;
  send_state("FINISH");
}

void reset_clock() {
  clockstate = 0;
  send_state("RESET");
}
