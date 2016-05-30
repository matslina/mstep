#pragma once

class Mode {
public:
  virtual void start() {}
  virtual void stop() {}
  virtual bool tick() = 0;
};

enum FieldType {FieldTypeInteger, FieldTypeNote};

class MultiFieldMode : public Mode {
private:
  char i;

protected:
  ProgramController *patternController;
  DisplayWriter *displayWriter;
  Control *control;
  Grid *grid;
  char numFields;
  const char **fieldName;
  char (ProgramController::**fieldFun)(char);
  FieldType *fieldType;
  bool rowSelectRequired;

public:
  void start() {
    i = 0;
    if (rowSelectRequired)
      displayWriter->clear()->string(fieldName[i])->cr()->string("  <select row>")->cr();
    else
      displayWriter->clear()->namedInteger(fieldName[i], (patternController->*fieldFun[i])(0));
  }

  void stop() {
    patternController->selectedRow = -1;
    patternController->draw();
  }

  bool tick() {
    bool updateDisplay = false;
    char row, col;

    if (rowSelectRequired) {
      row = -1;
      while (grid->getPress(&row, &col));
      if (row >= 0) {
	patternController->selectedRow = row;
	patternController->draw();
	updateDisplay = true;
      }

      if (patternController->selectedRow < 0)
	return true;
    }

    if (control->getSelect()) {
      if (++i >= numFields)
	i = 0;
      updateDisplay = true;
    }

    int mod = control->getMod();
    if (mod || updateDisplay) {
      displayWriter->clear()->string(fieldName[i])->cr()->string("  ");
      if (fieldType[i] == FieldTypeInteger)
	displayWriter->integer((patternController->*fieldFun[i])(mod));
      else
	displayWriter->note((patternController->*fieldFun[i])(mod));
      displayWriter->cr();
    }

    return true;
  }
};
