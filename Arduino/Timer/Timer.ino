#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <Bounce2.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include "config.h"

#define USE_SERIAL Serial

// "WeMos" WiFi&Bluetooth Battery, 80Mhz, 921600

////////////////////////////////////////////////////////////////////////////////
#define NUMPIXELS 43 // Number of LEDs in a strip (some are actually 56, 
//some 57 due to extra colon/decimal points)
#define DATAPIN0 14 //digit 0 NeoPixel 60 strip far RIGHT
#define DATAPIN1 32 //digit 1 (plus lower colon dot)
#define DATAPIN2 15 //digit 2 (plus upper colon dot)
#define DATAPIN3 33 //digit 3 (plus decimal dot)
#define DATAPIN4 27 //digit 4 far LEFT

Adafruit_NeoPixel strip[] = { //here is the variable for the multiple strips
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN0, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN1, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN2, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN3, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN4, NEO_GRB + NEO_KHZ800)
};
const int bright = 200; //adjust brightness for all pixels 0-255 range,
// 32 being pretty dim, 255 being full brightness

#define DEBOUNCE 10
#define DIGITS 5
#define FLASHINTERVAL 1000

#define RESETPIN 26
#define STARTPIN 25
#define PAUSEPIN 34
#define RESUMEPIN 39
#define STOPPIN 36

#define NUM_BUTTONS 5
const uint8_t BUTTON_PINS[NUM_BUTTONS] = { RESETPIN, STARTPIN, PAUSEPIN, RESUMEPIN, STOPPIN };

WiFiMulti WiFiMulti;
HTTPClient http;

Bounce * buttons = new Bounce[NUM_BUTTONS];
String pressed = "";
char display_time[DIGITS + 1] = "00000";
char last_display[DIGITS + 1] = "00000";

unsigned long course_time;
unsigned long last_flash;
unsigned long start_time;
int course_state = 0;
int clock_colour = 1;
int clock_flash = 0;
int clock_flash_state = LOW;

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

  // allow reuse (if server supports it)
  http.setReuse(true);
    
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
    digitWrite(t,0,0); //clear it
    strip[t].show();
    digitWrite(t,0,1); //display it
    strip[t].show();
  }
  delay(1500);
//END void setup()
////////////////////////////////////////////////////////////////////////////////
}


////////////////////////////////////////////////////////////////////////////////
void loop() {   
  for (int i=0; i < NUM_BUTTONS; i++) {
    buttons[i].update();
  }
  if (buttons[0].fell() ) {
    pressed = "RESET";
    course_time = 0;
    course_state = 0;
    clock_colour = 1;
    clock_flash = 0;
  }

  if (buttons[1].fell() ) {
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
    clock_colour = 2;
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
    clock_colour = 3;
    clock_flash = 1;
  }
  
  if (pressed != "") {
    send_state(pressed, course_time);
    pressed = "";
  }

  if (course_state == 1) {
    sprintf(display_time, "%05d", (millis() - start_time)/100); // Uses millis() and displays 1/10 of a second
  }
  if (course_state == 3) {
    sprintf(display_time, "%05d", ((millis() - start_time)+course_time)/100); // Uses millis() and displays 1/10 of a second
  }

  if (course_state == 1 || course_state == 3) { // Clock only runs in these two states
    for (int i=0; i<DIGITS; i++) {
      if (display_time[i] != last_display[i]) { // Only update a digit if it has changed.
        int digit = display_time[i] - '0'; 
        digitWrite(i, digit, clock_colour);
        strip[i].show();
        last_display[i] = display_time[i];
        USE_SERIAL.print(display_time);
        USE_SERIAL.print(" | Changing digit: ");
        USE_SERIAL.print(i);
        USE_SERIAL.print(" to: ");
        USE_SERIAL.println(digit);
      }
    }
  }

  if (clock_flash == 1) {
    unsigned long currentMillis = millis();
    if (currentMillis - last_flash >= FLASHINTERVAL) {
      last_flash = currentMillis;
      if (clock_flash_state == LOW) {
        for (int i=0; i<DIGITS; i++) {
          digitWrite(i, display_time[i] - '0', 0);
          strip[i].show();
          clock_flash_state = HIGH;
        }
      } else {
        for (int i=0; i<DIGITS; i++) {
          digitWrite(i, display_time[i] - '0', clock_colour);
          strip[i].show();
          clock_flash_state = LOW;
        }
      }
    }
  }
  
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
