#ifndef TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml
#define TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"


static const char *tmodeFieldName[] = \
  {"TEMPO"};

static char (PatternController::*tmodeFieldFun[])(char) = \
  {&PatternController::modTempo};


class TempoMode : public MultiFieldMode {
public:
  TempoMode(DisplayWriter *displayWriter, Control *control, PatternController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->patternController = pc;
    numFields = 1;
    fieldName = tmodeFieldName;
    fieldFun = tmodeFieldFun;
  }
};

#endif
