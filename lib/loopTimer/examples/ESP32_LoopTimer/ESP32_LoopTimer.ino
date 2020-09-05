// ESP32_LoopTimer.ino
/*
 * (c)2019 Forward Computing and Control Pty. Ltd.
 * NSW Australia, www.forward.com.au
 * This code is not warranted to be fit for any purpose. You may only use it at your own risk.
 * This generated code may be freely used for both private and commercial use
 * provided this copyright is maintained.
 */

#include <loopTimer.h>
#include <millisDelay.h>

#include <pfodBufferedStream.h>
// install pfodParser library from https://www.forward.com.au/pfod/pfodParserLibraries/index.html
// to get pfodBufferedStream

int led = 13;
const size_t bufSize = 300; // to allow for fault outputs
uint8_t buf[bufSize];
pfodBufferedStream bufferedStream(115200, buf, bufSize, false); // default is blocking == true, if set blocking arg == false, do not block just drop excess bytes
// with blocking true do not miss any output, but may block loop();

loopTimerClass blinkTaskTimer("blink");

bool ledOn = false;
millisDelay ledDelay;

void blinkTask(void* parameter) {
  ledOn = false;
  // start delay
  ledDelay.start(1000);

  for (;;) {   // loop forever
    blinkTaskTimer.check();
    vTaskDelay(1); // need this to prevent wdt panic
    
    if (ledDelay.justFinished()) {
      ledDelay.repeat(); // start delay again without drift
      // toggle the led
      ledOn = !ledOn;
      if (ledOn) {
        digitalWrite(led, HIGH); // turn led on
      } else {
        digitalWrite(led, LOW); // turn led off
      }
    }
  }
  /* delete a task when finish,
    this will never happen because this is infinity loop */
  vTaskDelete( NULL );
}

millisDelay printTimers;
const unsigned long PRINT_TIMERS_DELAY = 5000;

void setup() {
  Serial.begin(115200);
  for (int i = 10; i > 0; i--) {
    Serial.println(i);
    delay(500);
  }
  // initialize digital pin led as an output.
  pinMode(led, OUTPUT);

  xTaskCreatePinnedToCore(
    blinkTask,           /* Task function. */
    "blinkTask",        /* name of task. */
    1000,                    /* Stack size of task */
    NULL,                     /* parameter of the task */
    1,                        /* priority of the task */  // loop() has priority == 1
    NULL,                    /* Task handle to keep track of created task */
    CONFIG_ARDUINO_RUNNING_CORE); //pin to core 1, WiFi is pinned to 0

  printTimers.start(PRINT_TIMERS_DELAY);
  bufferedStream.connect(&Serial);
}


void loop() {
  loopTimer.check();
  bufferedStream.available(); // check if any output to print
  
  if (printTimers.justFinished()) {
    printTimers.restart(); // this may drift
    loopTimer.print(&bufferedStream);
    blinkTaskTimer.print(&bufferedStream);
  }
  vTaskDelay(1); // need this to prevent wdt panic
}
