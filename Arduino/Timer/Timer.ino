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

#ifdef USE_FASTLED
 CRGB strip[5][NUMPIXELS];
#else

Adafruit_NeoPixel strip[] = { //here is the variable for the multiple strips
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN0, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN1, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN2, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN3, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, DATAPIN4, NEO_GRB + NEO_KHZ800)
};
#endif
const int bright = 200; //adjust brightness for all pixels 0-255 range,
// 32 being pretty dim, 255 being full brightness

#define DEBOUNCE 10
#define DIGITS 5
#define FLASHINTERVAL 500

#define RESETPIN 26
#define STARTPIN 25
#define PAUSEPIN 34
#define RESUMEPIN 39
#define STOPPIN 36

#define CELEBRATION 10

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
  for(int t=0;t<5;t++){
    digitWrite(t,0,0); //clear it
    FastLED.show();
    digitWrite(t,0,1); //display it
    FastLED.show();
  }   
  
  #else
  //NeoPixel array setup
  for(int s=0;s<5;s++){
    strip[s].begin(); // Initialize pins for output
    strip[s].setBrightness(bright);
    strip[s].show();
    delay(200);
  }    
  //flash an zero
  for(int t=0;t<5;t++){
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
     if (timeout > 30) {
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
      color[1]={255};
      color[2]={0};
    }
    if (col==3){ //blue
      color[0]={0};
      color[1]={0};
      color[2]={255};
    }
    if (col==4){ //white -- careful with this one, 3x power consumption
      //if 255 is used
      color[0]={160};
      color[1]={160};
      color[2]={160};
    }

      if (col==5){ //yellow
      color[0]={255};
      color[1]={255};
      color[2]={0};
    }
        if (col==6){ // pink
      color[0]={255};
      color[1]={0};
      color[2]={255};
    }
        if (col==7){ // orange
      color[0]={255};
      color[1]={255};
      color[2]={0};
    }
        if (col==8){ // cyan
      color[0]={0};
      color[1]={255};
      color[2]={255};
    }
        if (col==9){ // purple
      color[0]={175};
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
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif
    }  
  }
  //seg A
  if(seg==2){
      //light second 8
      for(int i=6; i<12; i++){
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif
    } 
  }
  //seg B
  if(seg==3){
      for(int i=12; i<18; i++){
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif
      }   
  }
  //seg C
  if(seg==4){
      for(int i=18; i<24; i++){
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif
      }   
  }
  //seg D
  if(seg==5){
      for(int i=24; i<30; i++){
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif
      }   
  }
  //seg E
  if(seg==6){
      for(int i=30; i<36; i++){
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif
      }   
  }
  //seg G
  if(seg==7){
      for(int i=36; i<42; i++){
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif
      }   
  }
  //seg dp
  if(seg==8){
      for(int i=42; i<43; i++){
#ifdef USE_FASTLED
      strip[digit][i] = CRGB(color[0],color[1],color[2]);
#else
      strip[digit].setPixelColor(i,color[0],color[1],color[2]);
#endif
      }   
  }
}
//END void segLight()
////////////////////////////////////////////////////////////////////////////////
