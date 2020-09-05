/*
 * (c)2014-2017 Forward Computing and Control Pty. Ltd.
 * NSW Australia, www.forward.com.au
 * This code is not warranted to be fit for any purpose. You may only use it at your own risk.
 * This code may be freely used for both private and commercial use
 * Provide this copyright is maintained.
 */
#include "pfodTouchAction.h"
 

pfodTouchAction::pfodTouchAction()  {
}

void pfodTouchAction::init(Print *_out, struct VALUES* _values) {
 // initValues(_values);
  out = _out;
  actionCmd = ' ';
 // don't do this here else action(pfodDwgsBase &_action)  will force send
 // valuesPtr->lastDwg = this; 
 valuesPtr = _values;
}


pfodTouchAction &pfodTouchAction::cmd(const char _cmd) {
  actionCmd = _cmd;
  return *this;
}


pfodTouchAction &pfodTouchAction::action(pfodDwgsBase &_action) {
  actionPtr = &_action;
  return *this;
}

void pfodTouchAction::send(char _startChar) {
  out->print(_startChar);
  out->print('X');
  out->print('~');
  out->print(actionCmd);
  if (actionPtr != NULL) {
  	  actionPtr->send('~');
  }
 // valuesPtr->lastDwg = NULL; // sent now
}


