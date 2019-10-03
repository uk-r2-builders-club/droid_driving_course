#include <Bounce2.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

#include "config.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

const byte address_bits[] =  { 13,12,14,16 };
const int lighttime = 10000;
unsigned int address;
unsigned int localPort = 8888;
char packetBuffer[64]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged\r\n";       // a string to send back
int sensitivity = 11000;
int eeAddress = 0; 
bool inCalibration = false;

#define CELEBRATION     10
#define LED_PIN         3                        // Neopixel string data in pin
#define LED_COUNT       8                        // Number of neopixels
#define CALIBRATION_PIN 16                       // Input pin for push button on rotary encoder
#define encoder0PinA    2                        // Input for one direction of rotary encoder
#define encoder0PinB    0                        // Input pin for the other direction of the rotary encoder
#define SENSE_SCALE     100                      // How much sensitivity will change per click
#define SENSE_MIN       5000                     // Minimum sensitivity
#define SENSE_MAX       13000                    // Maximum sensitivity
#define MAG_MID         5000                     // Magnitude mid point
#define SHOCK_INTERVAL  500                      // How often the shock peak will decrease
#define SHOCK_DECREASE  1000                     // How much the shock peak will decrease with each SHOCK_INTERVAL

MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int32_t shock_peak;
int32_t shock_show;
long last_peak;

WiFiUDP Udp;
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
HTTPClient http;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t led_red    = strip.Color(255,0,0);
uint32_t led_orange = strip.Color(255,165,0);
uint32_t led_green  = strip.Color(0,255,0);
uint32_t led_blue   = strip.Color(0,0,255);
uint32_t led_off    = strip.Color(0,0,0);

Bounce debouncer = Bounce();
void ICACHE_RAM_ATTR doEncoderA();
void ICACHE_RAM_ATTR doEncoderB();

long timer = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");
  pinMode(CALIBRATION_PIN,INPUT_PULLUP);
  debouncer.attach(CALIBRATION_PIN);
  debouncer.interval(25);

  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);

  // encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);

  // encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);
  
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
  address = address + 16; // Shock sensors are in the second block of 16 sensors.
  Serial.print("Address: ");
  Serial.println(address);

  strip.begin();
  strip.setBrightness(70);
  strip.show();
 

  // Check for a stored value for sensitivity, and read it. Or use the default if it isn't present
  int s;
  Serial.print("EEPROM Stored sensitivity: ");
  EEPROM.begin(512);
  EEPROM.get(eeAddress, s);
  Serial.println(s);
  if (s > SENSE_MIN && s < SENSE_MAX) {
    Serial.println("Setting sensitivity from EEPROM");
    sensitivity = s;
  } else {
    Serial.println("Storing default sensitivity in EEPROM");
    EEPROM.put(eeAddress, sensitivity);
    EEPROM.commit();
  }
  
  char sensor_name[20];
  sprintf(sensor_name, "shock_sensor%02d", address);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(sensor_name);
  WiFiMulti.addAP(ssid, pass);

  int timeout = 0;
  while (WiFiMulti.run() != WL_CONNECTED) {
     delay(100);
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
     strip.fill(strip.Color(0, 0, 0),0,LED_COUNT);
     strip.show();
  } else {
     Serial.println("TIMEOUT!");
     flashLights(10, 100);
     strip.fill(strip.Color(0, 0, 0),0,LED_COUNT);
     strip.show();     
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


  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif
 
  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  delay(500);
  
  accelgyro.setXAccelOffset(1250);
  accelgyro.setYAccelOffset(500);
  accelgyro.setZAccelOffset(-16000);  
  
}

void loop() {
  debouncer.update();

  if ( debouncer.fell() ) {  // Call code if button transitions from HIGH to LOW
     inCalibration = !inCalibration;
     startupLights();
     Serial.println("Toggle calibration mode");     
     EEPROM.put(eeAddress, sensitivity);
     EEPROM.commit();
  }

  if (millis() > last_peak + SHOCK_INTERVAL) {
    shock_peak = shock_peak - SHOCK_DECREASE;
    shock_leds();
    last_peak = millis();
  }
  
  if (!inCalibration) {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    long int mag = (sqrt((ax * ax)/10 + (ay * ay)/10 + (az * az)/10)) - MAG_MID;
    if (mag > shock_peak) {
      shock_peak = mag;
      last_peak = millis();
      int lit_leds = mag/((sensitivity)/LED_COUNT);
      shock_leds();
    }
    
    Serial.print("Magnitude: ");
    Serial.print(mag);

    if (mag < 0 ) {
      mag = mag * -1;
    }

    if (mag > sensitivity) {
      Serial.println("SHOCK!!!!!!!!");
      Serial.print("Magnitude: ");
      Serial.println(mag);
      String api_call = String((char*)api) + "gate/" + address + "/FAIL";
      Serial.print("API Call: ");
      Serial.println(api_call);
      http.begin(client, api_call);
      int httpCode = http.GET();
      http.end();      
      delay(5000);
      Serial.println("Continuing monitoring");
    }
    Serial.print("/");
    Serial.print(mag);
    Serial.print("\t");
    Serial.print(ax); Serial.print("\t");
    Serial.print(ay); Serial.print("\t");
    Serial.print(az); Serial.print("\t");
    Serial.print(shock_peak); Serial.print("\t");
    Serial.println(shock_show); 
  } else {
    // Calculate the number of LEDs to light
    int lit_leds = (sensitivity - SENSE_MIN)/((SENSE_MAX-SENSE_MIN)/LED_COUNT);
    
    Serial.print("Sensitivity: ");
    Serial.print(sensitivity);
    Serial.print(" LEDs to light: ");
    Serial.println(lit_leds);
    calibrate_leds(lit_leds);
  }
}

void calibrate_leds(int lit_leds) {
  // Light up 'lit_leds' to show calibration level
  for (int i = 0; i < LED_COUNT; i++) {
    if (i < lit_leds) {
      strip.setPixelColor(i, led_blue);
    } else {
      strip.setPixelColor(i, led_off);
    }
  }
  strip.show();
}

void shock_leds() {
    int lit_leds = shock_peak/((sensitivity)/LED_COUNT);
    int l;
    for (l = 0 ; l < LED_COUNT; l++) {
      uint32_t colour = led_green;       // Default to LED being green
      if (l > (LED_COUNT/2) - 1) {       // If it is over half way, make the LED orange
        colour = led_orange;
      }
      if (l == LED_COUNT - 1) {           // If its the final LED then it should be red
        colour = led_red;
      }
      if (l < lit_leds) {
        strip.setPixelColor(l, colour);
      } else {
        strip.setPixelColor(l, led_off);
      }
    }
    strip.show(); 
}

void changePos(int x) {
  sensitivity = (sensitivity+(x*SENSE_SCALE));
  if (sensitivity > SENSE_MAX)
      sensitivity = SENSE_MAX;
  if (sensitivity < SENSE_MIN)
      sensitivity = SENSE_MIN;
}

void doEncoderA() {
  // look for a low-to-high on channel A
  if (digitalRead(encoder0PinA) == HIGH) {
    // check channel B to see which way encoder is turning
    if (digitalRead(encoder0PinB) == LOW) 
      changePos(1);          // CW
    else 
      changePos(-1);         // CCW
  } else {
    // must be a high-to-low edge on channel A
    // check channel B to see which way encoder is turning
    if (digitalRead(encoder0PinB) == HIGH) 
      changePos(1);          // CW
    else 
      changePos(-1);         // CCW
  }
}

void doEncoderB() {
  // look for a low-to-high on channel B
  if (digitalRead(encoder0PinB) == HIGH) {
    // check channel A to see which way encoder is turning
    if (digitalRead(encoder0PinA) == HIGH) 
      changePos(1);          // CW
    else 
      changePos(-1);         // CCW
  } else {
    // Look for a high-to-low on channel B
    // check channel B to see which way encoder is turning
    if (digitalRead(encoder0PinA) == LOW) 
      changePos(1);          // CW
    else 
      changePos(-1);         // CCW
  }
}


void flashLights(int count, int wait) {
  int i;
  for(i=0; i<count; i++) {
     strip.fill(strip.Color(255,0,0),0,LED_COUNT);
     strip.show();
     delay(wait);
     strip.fill(strip.Color(0,0,0),0,LED_COUNT);
     strip.show();
     delay(wait);
  }
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void pulseLights() {
   uint16_t i, j;
   for(i=0; i<50; i++) {
     strip.fill(strip.Color(i*5,0,0), 0, LED_COUNT);
     strip.show();
     delay(5);
   }
   for(j=50; j>0; j--) {
     strip.fill(strip.Color(j*5,0,0), 0, LED_COUNT);
     strip.show();
     delay(5);
   }  
}

void startupLights() {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<9; i++) {
      strip.fill(Wheel((i+j) & 255),0,LED_COUNT);
    }
    strip.show();
    delay(5);
  }
  strip.clear();
  strip.show();
}
