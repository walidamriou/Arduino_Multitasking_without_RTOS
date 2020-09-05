// TempInputBlink_Tasks.ino
// Uses MAX31856_noDelay library which does not use delay(), (good)
/*
   (c)2019 Forward Computing and Control Pty. Ltd.
   NSW Australia, www.forward.com.au
   This code is not warranted to be fit for any purpose. You may only use it at your own risk.
   This generated code may be freely used for both private and commercial use
   provided this copyright is maintained.
*/
#include <loopTimer.h>
// install the loopTimer library from https://www.forward.com.au/pfod/ArduinoProgramming/RealTimeArduino/TimingDelaysInArduino.html
// loopTimer.h also needs the millisDelay library installed from https://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html
#include <pfodBufferedStream.h>
#include <pfodNonBlockingInput.h>
// install pfodParser library from https://www.forward.com.au/pfod/pfodParserLibraries/index.html
// to get pfodBufferedStream and pfodNonBlockingInput
#include <millisDelay.h>

#include <MAX31856_noDelay.h>
// Use software SPI: CS, DI, DO, CLK
MAX31856_noDelay maxthermo = MAX31856_noDelay(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
//MAX31856_noDelay maxthermo = MAX31856_noDelay(10);

#define BAUD_RATE 9600
//Example of using bufferedStream to release bytes at BAUD_RATE baud, buffer size 130
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

bool stopTempReadings = true;
float tempReading = 0.0;
millisDelay max31856Delay;
const unsigned long MAX31856_DELAY_MS = 200; // max single shot conversion time is 185mS
bool readingStarted = false;


// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(BAUD_RATE);
  for (int i = 10; i > 0; i--) {
    Serial.println(i);
    delay(500);
  }
  nonBlocking.connect(bufferedStream.connect(&Serial));  // connect nonBlocking to a buffered stream to Serial

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  maxthermo.begin();
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);

  ledDelay.start(1000); // start the ledDelay, toggle every 1000mS
  printDelay.start(5000); // start the printDelay, print every 5000mS
  bufferedStream.println(F("Enter any char to start reading Temp:"));
  nonBlocking.clearInput(); // clear out any old input waiting to be read.  This call is non-blocking
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
    if (stopTempReadings) {
      bufferedStream.println(F("Temp reading stopped"));
    } else {
      bufferedStream.print(F("Temp:")); bufferedStream.println(tempReading);
    }
  } // else nothing to do this call just return, quickly
}

// task to get the first char user enters, input terminated by \r or \n
// return 0 if nothing input
char getInput() {
  char rtn = 0;
  int inputLen = nonBlocking.readInputLine(lineBuffer, lineBufferSize, true); // echo input
  if (inputLen > 0) {
    // got some user input either 9 chars or less than 9 chars terminated by \r or \n
    rtn = lineBuffer[0]; // collect first char for rtn.
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


// the loop function runs over and over again forever
void loop() {
  loopTimer.check(&nonBlocking);
  char in = getInput(); // call input task, this also releases buffered prints
  if (in != 0) {
    bufferedStream.print(F("User entered:")); bufferedStream.println(in);
    if (in != 's') {
      stopTempReadings = false;
    } else {
      stopTempReadings = true;
    }
    lineBuffer[0] = '\0'; // clear buffer for reuse
    // prompt user again
    if (stopTempReadings) {
      bufferedStream.println(F("Enter any char to start reading Temp:"));
    } else {
      bufferedStream.println(F("Enter s to stop reading Temp:"));
    }
    nonBlocking.clearInput(); // clear out any old input waiting to be read.  This call is non-blocking
  }
  blinkLed7(stopTempReadings); // call the method to blink the led
  printTemp(); // print the temp
  if (!stopTempReadings) {
    int rtn = readTemp(); // check for errors here
  }
}
