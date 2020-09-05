#include <Arduino.h>
#include <loopTimer.h>

int led = 13;

bool led_On = false; 
millisDelay led_Delay;
millisDelay print_Delay;

void task1();
void task2();

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  
  // start the ledDelay, toggle every 1000mS
  led_Delay.start(1000); 

  // start the printDelay, print every 5000mS
  print_Delay.start(5000); 
}

void loop() {
  // Tracker of the maximum and average time it take to run the loop code
  loopTimer.check(&Serial); // print every 5s

  task1(); 
  task2();
}

// task 1 is to blink the led
void task1(){
  if (led_Delay.justFinished()) {   // check if delay has timed out
    led_Delay.repeat(); // start delay again without drift
    led_On = !led_On;     // toggle the led
    digitalWrite(led, led_On?HIGH:LOW); // turn led on/off
  } // else nothing to do this call just return, quickly
}

// task 2 is to print the current time in ms every 5s
void task2(){
  if (print_Delay.justFinished()) {
    print_Delay.repeat(); // start delay again without drift
    Serial.println(millis());   // print the current mS
  } // else nothing to do this call just return, quickly
}