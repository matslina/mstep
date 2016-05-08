#ifndef TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml
#define TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"


static const char *tmodeFieldName[] = \
  {"TEMPO"};

static char (ProgramController::*tmodeFieldFun[])(char) = \
  {&ProgramController::modTempo};


class TempoMode : public MultiFieldMode {
public:
  TempoMode(DisplayWriter *displayWriter, Control *control, ProgramController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->patternController = pc;
    numFields = 1;
    fieldName = tmodeFieldName;
    fieldFun = tmodeFieldFun;
  }
};

#endif
