// ESP32DamperControl.ino
// User input is now via a terminal program connecting with Classic Bluetooth.
// When you connect just send an empty line to get the prompt.
/*
   (c)2019 Forward Computing and Control Pty. Ltd.
   NSW Australia, www.forward.com.au
   This code is not warranted to be fit for any purpose. You may only use it at your own risk.
   This generated code may be freely used for both private and commercial use
   provided this copyright is maintained.
*/
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

#include <loopTimer.h>
// install the loopTimer library from https://www.forward.com.au/pfod/ArduinoProgramming/RealTimeArduino/TimingDelaysInArduino.html
// loopTimer.h also needs the millisDelay library installed from https://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html
#include <pfodBufferedStream.h>
#include <pfodNonBlockingInput.h>
// install pfodParser library from https://www.forward.com.au/pfod/pfodParserLibraries/index.html
// to get pfodBufferedStream and pfodNonBlockingInput
#include <AccelStepper.h>
#include <millisDelay.h>

#include <MAX31856_noDelay.h>
// Use software SPI: CS, DI, DO, CLK
MAX31856_noDelay maxthermo = MAX31856_noDelay(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
//MAX31856_noDelay maxthermo = MAX31856_noDelay(10);

#define BAUD_RATE 115200
//Example of using bufferedStream to release bytes at BAUD_RATE, buffer size 130
const size_t bufSize = 130;
uint8_t buf[bufSize];
pfodBufferedStream bufferedStream(BAUD_RATE, buf, bufSize);

pfodNonBlockingInput nonBlocking;
const size_t lineBufferSize = 10; // 9 + terminating null
char lineBuffer[lineBufferSize] = ""; // start empty

int led = 7; // new pin for led
// Pin 13 is used for the MAX31856 board
bool ledOn = false; // keep track of the led state
millisDelay ledDelay;
millisDelay printDelay;

float tempReading = 0.0; // from readTemp
float simulatedTempReading = 0.0; // from user input
bool closeDampler = true;

millisDelay max31856Delay;
const unsigned long MAX31856_DELAY_MS = 200; // max single shot conversion time is 185mS
bool readingStarted = false;

AccelStepper stepper; // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(BAUD_RATE);
  for (int i = 10; i > 0; i--) {
    Serial.println(i);
    delay(500);
  }
  SerialBT.begin("DamperCtrl"); //Bluetooth device name
  nonBlocking.connect(bufferedStream.connect(&SerialBT));  // connect nonBlocking to a buffered stream to SerialBT

  //initialize digital pin led as an output.
  pinMode(led, OUTPUT);

  maxthermo.begin();
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);

  ledDelay.start(1000); // start the ledDelay, toggle every 1000mS
  printDelay.start(5000); // start the printDelay, print every 5000mS
  bufferedStream.println(F("Enter 0 to 5 to simulate 0 to 100degs, any other char to close damper."));
  nonBlocking.clearInput(); // clear out any old input waiting to be read.  This call is non-blocking

  stepper.setMaxSpeed(1000);
  stepper.setSpeed(500); // need to call atleast every 2mS
  stepper.setAcceleration(50);
  Serial.println("The device started, now you can pair it with Classic bluetooth!");
}

// the task method
void blinkLed7(bool stop) {
  if (ledDelay.justFinished()) {   // check if delay has timed out
    ledDelay.repeat(); // start delay again without drift
    if (stop) {
      digitalWrite(led, LOW); // turn led on/off
      ledOn = false;
      return;
    }
    ledOn = !ledOn;     // toggle the led
    digitalWrite(led, ledOn ? HIGH : LOW); // turn led on/off
  } // else nothing to do this call just return, quickly
}

// the task method
void printTemp() {
  if (printDelay.justFinished()) {
    printDelay.repeat(); // start delay again without drift
    bufferedStream.print(F("Temp:")); bufferedStream.println(simulatedTempReading);
    bufferedStream.print(F("Position current:")); bufferedStream.println(stepper.currentPosition());
  } // else nothing to do this call just return, quickly
}

// task to get the first char user enters, input terminated by \r or \n
// return 0 if nothing input
char getInput() {
  char rtn = 0;
  int inputLen = nonBlocking.readInputLine(lineBuffer, lineBufferSize, true); // echo input
  if (inputLen >= 0) {
    // got some user input either 9 chars or less than 9 chars terminated by \r or \n
    if (inputLen == 0) {
      // empty line
      rtn = '\n';
    } else {
      rtn = lineBuffer[0]; // collect first char for rtn.
    }
  }
  return rtn;
}

// return 0 if have new reading and no errors
// returns -1 if no new reading
// returns >0 if have errors
int readTemp() {
  if (!readingStarted) { // start one now
    maxthermo.oneShotTemperature();
    // start delay to pick up results
    max31856Delay.start(MAX31856_DELAY_MS);
  }
  if (max31856Delay.justFinished()) {
    readingStarted = false;
    // can pick up both results now
    tempReading = maxthermo.readThermocoupleTemperature();
    return 0; // new reading
  }
  return -1; // no new reading
}

void setDamperPosition() {
  if (closeDampler) {
    stepper.moveTo(0);
  } else {
    long stepPosition = simulatedTempReading * 50;
    stepper.moveTo(stepPosition);
  }
}

void runStepper() {
  loopTimer.check(&bufferedStream); // moved here from loop()
  stepper.run();
}

// the loop function runs over and over again forever
void loop() {
  //  loopTimer.check(&bufferedStream); move to runStepper() to check how often it is called
  char in = getInput(); // call input task, this also releases buffered prints
  if (in != 0) {
    if (in == '\n') {
      bufferedStream.println(F("Enter 0 to 5 to simulate 0 to 100degs, any other char to close damper."));
    } else {
      bufferedStream.print(F("Cmd Entered:")); bufferedStream.println(in);
      closeDampler = false;
      if (in == '0') {
        simulatedTempReading = 0.0;
      } else if (in == '1') {
        simulatedTempReading = 20.0;
      } else if (in == '2') {
        simulatedTempReading = 40.0;
      } else if (in == '3') {
        simulatedTempReading = 60.0;
      } else if (in == '4') {
        simulatedTempReading = 80.0;
      } else if (in == '5') {
        simulatedTempReading = 100.0;
      } else {
        closeDampler = true;
        bufferedStream.println(F("Close Damper"));
      }
      lineBuffer[0] = '\0'; // clear buffer for reuse
      // prompt user again
      nonBlocking.clearInput(); // clear out any old input waiting to be read.  This call is non-blocking
    }
  }
  blinkLed7(closeDampler); // call the method to blink the led
  runStepper(); // <<<< extra call here
  printTemp(); // print the temp
  runStepper(); // <<<< extra call here
  int rtn = readTemp(); // check for errors here
  setDamperPosition();
  runStepper();
}
