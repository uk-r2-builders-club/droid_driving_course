#include <NewPing.h>
 
#define TRIGGER_PIN  0
#define ECHO_PIN     2
#define MAX_DISTANCE 80
#define OUTPUT_PIN   1
 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
 
void setup() {
  pinMode(OUTPUT_PIN, OUTPUT);
}
 
void loop() {
  delay(50);
  
  if (sonar.ping_cm() < 20) {
     digitalWrite(OUTPUT_PIN, HIGH);
  } else {
     digitalWrite(OUTPUT_PIN, LOW);
  }
}
