#pragma once

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "programcontroller.hpp"

static const char *nmodeFieldName[] = \
  {"NOTE",
   "VELOCITY"};

static char (ProgramController::*nmodeFieldFun[])(char) = \
  {&ProgramController::modNote,
   &ProgramController::modVelocity};

static FieldType nmodeFieldType[] = {FieldTypeNote, FieldTypeInteger};

class NoteMode : public MultiFieldMode {
public:
  NoteMode(DisplayWriter *displayWriter, Control *control, Grid *grid,
	   ProgramController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->grid = grid;
    this->patternController = pc;
    numFields = 2;
    fieldName = nmodeFieldName;
    fieldFun = nmodeFieldFun;
    fieldType = nmodeFieldType;
    rowSelectRequired = true;;
  }
};
