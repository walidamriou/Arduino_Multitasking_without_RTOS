/*
 * (c)2014-2017 Forward Computing and Control Pty. Ltd.
 * NSW Australia, www.forward.com.au
 * This code is not warranted to be fit for any purpose. You may only use it at your own risk.
 * This code may be freely used for both private and commercial use
 * Provide this copyright is maintained.
 */
#include "pfodInsertDwg.h"

pfodInsertDwg::pfodInsertDwg()  {
}

void pfodInsertDwg::init(Print *_out, struct VALUES* _values) {
  initValues(_values);
  out = _out;
  //valuesPtr->lastDwg = this;
}

pfodInsertDwg &pfodInsertDwg::offset(float _colOffset, float _rowOffset) {
  valuesPtr->colOffset = _colOffset;
  valuesPtr->rowOffset = _rowOffset;
  return *this;
}

pfodInsertDwg &pfodInsertDwg::loadCmd(const char _loadCmd) {
  valuesPtr->cmd = _loadCmd;
  return *this;
}

void pfodInsertDwg::send(char _startChar) {
  out->print(_startChar);
  out->print('d');
  if (valuesPtr->centered != 0) {
    out->print('c'); // no centered yet
  }
  printIdx(); // no idx yet
  out->print('~');
  out->print(valuesPtr->cmd);
  out->print('~');
  out->print(""); // no cmd prefix for now
  colRowOffset();
}

