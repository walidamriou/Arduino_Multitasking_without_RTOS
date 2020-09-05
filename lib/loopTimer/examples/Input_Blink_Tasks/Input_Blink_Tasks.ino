// Input_Blink_Tasks.ino
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

#define BAUD_RATE 9600
//Example of using bufferedStream to release bytes at BAUD_RATE baud, buffer size 130
const size_t bufSize = 130;
uint8_t buf[bufSize];
pfodBufferedStream bufferedStream(BAUD_RATE, buf, bufSize);

pfodNonBlockingInput nonBlocking;
const size_t lineBufferSize = 10; // 9 + terminating null
char lineBuffer[lineBufferSize] = ""; // start empty

int led = 13;
// Pin 13 has an led connected on most Arduino boards.
bool ledOn = false; // keep track of the led state
millisDelay ledDelay;
millisDelay printDelay;
bool stopBlinking = false;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(BAUD_RATE);
  for (int i = 10; i > 0; i--) {
    Serial.println(i);
    delay(500);
  }
  nonBlocking.connect(bufferedStream.connect(&Serial));  // connect nonBlocking to a buffered stream to Serial

  // initialize digital pin led as an output.
  pinMode(led, OUTPUT);
  ledDelay.start(1000); // start the ledDelay, toggle every 1000mS
  printDelay.start(5000); // start the printDelay, print every 5000mS
  bufferedStream.println(F("Enter s to stop Led Blinking:"));
  nonBlocking.clearInput(); // clear out any old input waiting to be read.  This call is non-blocking
}

// the task method
void blinkLed13(bool stop) {
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
void print_mS() {
  if (printDelay.justFinished()) {
    printDelay.repeat(); // start delay again without drift
    bufferedStream.println(millis());   // print the current mS
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

// the loop function runs over and over again forever
void loop() {
  loopTimer.check(&nonBlocking);
  char in = getInput(); // call input task, this also releases buffered prints
  if (in != 0) {
    bufferedStream.print(F("User entered:")); bufferedStream.println(in);
    if (in != 's') {
      stopBlinking = false;
    } else {
      stopBlinking = true;
    }
    lineBuffer[0] = '\0'; // clear buffer for reuse
    // prompt user again
    if (stopBlinking) {
      bufferedStream.println(F("Enter any char to start Led Blinking:"));
    } else {
      bufferedStream.println(F("Enter s to stop Led Blinking:"));
    }
    nonBlocking.clearInput(); // clear out any old input waiting to be read.  This call is non-blocking
  }
  blinkLed13(stopBlinking); // call the method to blink the led
  print_mS(); // print the time
}
