#pragma once

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "programcontroller.hpp"


static const char *tmodeFieldName[] = \
  {"TEMPO"};

static char (ProgramController::*tmodeFieldFun[])(char) = \
  {&ProgramController::modTempo};

static FieldType tmodeFieldType[] = {FieldTypeInteger};

class TempoMode : public MultiFieldMode {
public:
  TempoMode(DisplayWriter *displayWriter, Control *control, Grid *grid, ProgramController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->patternController = pc;
    this->grid = grid;
    numFields = 1;
    fieldName = tmodeFieldName;
    fieldFun = tmodeFieldFun;
    fieldType = tmodeFieldType;
    rowSelectRequired = false;
  }
};
