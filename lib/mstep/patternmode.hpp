#ifndef PATTERNMODE_HPP_98387245kjdrkjfdsheiudsjdskj
#define PATTERNMODE_HPP_98387245kjdrkjfdsheiudsjdskj

#include "mstep.hpp"
#include "mode.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"

static const char *pmodeFieldName[] = \
  {"PATTERN",
   "SWING",
   "CHANNEL"};

static char (PatternController::*pmodeFieldFun[])(char) = \
  {&PatternController::modPattern,
   &PatternController::modSwing,
   &PatternController::modChannel};


class PatternMode : public MultiFieldMode {
public:
  PatternMode(DisplayWriter *displayWriter, Control *control, PatternController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->patternController = pc;
    numFields = 3;
    fieldName = pmodeFieldName;
    fieldFun = pmodeFieldFun;
  }
};

#endif
