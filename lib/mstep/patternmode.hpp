#pragma once

#include "mstep.hpp"
#include "mode.hpp"
#include "displaywriter.hpp"
#include "programcontroller.hpp"

static const char *pmodeFieldName[] = \
  {"PATTERN",
   "SWING",
   "CHANNEL"};

static char (ProgramController::*pmodeFieldFun[])(char) = \
  {&ProgramController::modPattern,
   &ProgramController::modSwing,
   &ProgramController::modChannel};


class PatternMode : public MultiFieldMode {
public:
  PatternMode(DisplayWriter *displayWriter, Control *control, ProgramController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->patternController = pc;
    numFields = 3;
    fieldName = pmodeFieldName;
    fieldFun = pmodeFieldFun;
  }
};
