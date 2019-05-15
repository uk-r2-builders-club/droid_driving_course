#include <NewPing.h>
 
#define TRIGGER_PIN   0      // PIN for the TRIG connection on the SR04
#define ECHO_PIN      2      // PIN for the ECHO connection on the SR04
#define MAX_DISTANCE  80     // Max distance likely to be sensed (speeds up routine)
#define OUTPUT_PIN    1      // Pin to output gate event on
#define GATE_INTERVAL 5000   // How long should the gate event last
 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
unsigned long last_trigger = 0;
 
void setup() {
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, HIGH);
}
 
void loop() {
  delay(50);
  
  unsigned int distance = sonar.ping_cm();
  
  if (distance < 60 && distance != 0) {
     digitalWrite(OUTPUT_PIN, LOW);
     last_trigger = millis();
  } else if ((millis() - last_trigger) > GATE_INTERVAL) {
     digitalWrite(OUTPUT_PIN, HIGH);
  }

}
