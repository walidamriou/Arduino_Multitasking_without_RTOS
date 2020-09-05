
/* ===== pfod Command for Menu_1 ====
pfodApp msg {.} --> {,<bg bl><b><+5>~ESP32 Led
<-5>(Sparkfun ESP32 Thing)`0~V11|A`0~Led ~%`255`0~100~0~}
 */
// Using ESP32 based board programmed via Arduino IDE
// follow the steps given on http://www.forward.com.au/pfod/ESP32/index.html to install ESP32 support for Arduino IDE
/* Code generated by pfodDesignerV3 V3.0.3305
 */
/*
 * (c)2014-2018 Forward Computing and Control Pty. Ltd.
 * NSW Australia, www.forward.com.au
 * This code is not warranted to be fit for any purpose. You may only use it at your own risk.
 * This generated code may be freely used for both private and commercial use
 * provide this copyright is maintained.
 */

#include "BluetoothSerial.h"
#define DEBUG

// Download pfodESP32BufferedClient library from http://www.forward.com.au/pfod/pfodParserLibraries/index.html
#include <pfodESP32Utils.h>
// download the libraries from http://www.forward.com.au/pfod/pfodParserLibraries/index.html
// pfodParser.zip V3.26+ contains pfodParser, pfodSecurity, pfodBLEBufferedSerial, pfodSMS and pfodRadio
#include <pfodParser.h>

#include <pfodBLEBufferedSerial.h> // used to prevent flooding bluetooth sends
int swap01(int); // method prototype for slider end swaps

pfodParser parser("V12"); // create a parser with menu version string to handle the pfod messages
pfodBLEBufferedSerial bufferedSerial;   // 1024 buffer size

const char DEVICE_NAME[]  = "ESP32_BT"; // <<<<< set your device name here
BluetoothSerial SerialBT;

// give the board pins names, if you change the pin number here you will change the pin controlled
int cmd_A_var; // name the variable for 'Led'
const int cmd_A_pin = 5; // name the output 5 for 'Led'
const int cmd_A_channel = 0; // channel to use for this PWM output 

// the setup routine runs once on reset:
void setup() {
  cmd_A_var = 0;
  ledcAttachPin(cmd_A_pin,cmd_A_channel); // assign pin to channel
  ledcSetup(cmd_A_channel, 490, 8); // 490hz PWM, 8-bit resolution to match Arduino Uno
  pfodESP32Utils::analogWrite(cmd_A_channel,cmd_A_var); // set PWM output

#ifdef DEBUG
  Serial.begin(115200);
  Serial.println();
#endif

  SerialBT.begin(DEVICE_NAME); //Bluetooth device name

#ifdef DEBUG
  Serial.println("Bluetooth started, now you can pair it (no pin needed)");
#endif

  parser.connect(bufferedSerial.connect(&SerialBT)); // connect the parser to the i/o stream via buffer


  // <<<<<<<<< Your extra setup code goes here
}

// the loop routine runs over and over again forever:
void loop() {
  uint8_t cmd = parser.parse(); // parse incoming data from connection
  // parser returns non-zero when a pfod command is fully parsed
  if (cmd != 0) { // have parsed a complete msg { to }
    uint8_t* pfodFirstArg = parser.getFirstArg(); // may point to \0 if no arguments in this msg.
    pfod_MAYBE_UNUSED(pfodFirstArg); // may not be used, just suppress warning
    long pfodLongRtn; // used for parsing long return arguments, if any
    pfod_MAYBE_UNUSED(pfodLongRtn); // may not be used, just suppress warning
    if ('.' == cmd) {
      // pfodApp has connected and sent {.} , it is asking for the main menu
      if (!parser.isRefresh()) {
        sendMainMenu(); // send back the menu designed
      } else {
        sendMainMenuUpdate(); // menu is cached just send update
      }

    // now handle commands returned from button/sliders
    } else if('A'==cmd) { // user moved PWM slider -- 'Led'
      // in the main Menu of Menu_1 
      parser.parseLong(pfodFirstArg,&pfodLongRtn); // parse first arg as a long
      cmd_A_var = (int)pfodLongRtn; // set variable
      pfodESP32Utils::analogWrite(cmd_A_channel,cmd_A_var); // set PWM output
      sendMainMenuUpdate(); // always send back a pfod msg otherwise pfodApp will disconnect.

    } else if ('!' == cmd) {
      // CloseConnection command
      closeConnection(parser.getPfodAppStream());
    } else {
      // unknown command
      parser.print(F("{}")); // always send back a pfod msg otherwise pfodApp will disconnect.
    }
  }
  //  <<<<<<<<<<<  Your other loop() code goes here 
  
}

void closeConnection(Stream *io) {
    // nothing special here
}

void sendMainMenu() {
  // !! Remember to change the parser version string
  //    every time you edit this method
  parser.print(F("{,"));  // start a Menu screen pfod message
  // send menu background, format, prompt, refresh and version
  parser.print(F("<bg bl><b><+5>~ESP32 Led\n<-5>(Sparkfun ESP32 Thing)`0"));
  parser.sendVersion(); // send the menu version 
  // send menu items
  parser.print(F("|A"));
  parser.print('`');
  parser.print(cmd_A_var); // output the current PWM setting
  parser.print(F("~Led ~%`255`0~100~0~"));
  parser.print(F("}"));  // close pfod message
}

void sendMainMenuUpdate() {
  parser.print(F("{;"));  // start an Update Menu pfod message
  // send menu items
  parser.print(F("|A"));
  parser.print('`');
  parser.print(cmd_A_var); // output the current PWM setting
  parser.print(F("}"));  // close pfod message
  // ============ end of menu ===========
}

int swap01(int in) {
  return (in==0)?1:0;
}
// ============= end generated code =========
 
