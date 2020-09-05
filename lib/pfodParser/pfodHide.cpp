/*
 * (c)2014-2017 Forward Computing and Control Pty. Ltd.
 * NSW Australia, www.forward.com.au
 * This code is not warranted to be fit for any purpose. You may only use it at your own risk.
 * This code may be freely used for both private and commercial use
 * Provide this copyright is maintained.
 */
#include "pfodHide.h"

pfodHide::pfodHide()  {
}

void pfodHide::init(Print *_out, struct VALUES* _values) {
  initValues(_values);
  out = _out;
  //valuesPtr->lastDwg = this;
}

pfodHide &pfodHide::idx(uint16_t _idx) {
  valuesPtr->idx = _idx;
  return *this;
}
pfodHide &pfodHide::cmd(const char _cmd) {
  valuesPtr->cmd = _cmd;
  return *this;
}

void pfodHide::send(char _startChar) {
  out->print(_startChar);
  out->print('h');
  if (valuesPtr->idx > 0) {
    printIdx();
  } else {
    out->print('~');
    out->print(valuesPtr->cmd);
  }  	  
 // valuesPtr->lastDwg = NULL; // sent now
}


