#include <NewPing.h>
 
#define TRIGGER_PIN  0
#define ECHO_PIN     2
#define MAX_DISTANCE 80
 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
 
void setup() {
}
 
void loop() {
  delay(50);
  
  if (sonar.ping_cm() < 40) {
     digitalWrite(1, HIGH);
  } else {
     digitalWrite(1, LOW);
  }
}
