#include <Arduino.h>
#include <loopTimer.h>

void setup() {
    Serial.begin(115200);
}

void loop() {
  // Track of the maximum and average time it take to run the loop code
   loopTimer.check(&Serial);
}