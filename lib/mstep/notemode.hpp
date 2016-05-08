#ifndef NOTEMODE_HPP_94lkdvnmxcsvslkfa9jseuixckmsd
#define NOTEMODE_HPP_94lkdvnmxcsvslkfa9jseuixckmsd

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"

class NoteMode : public Mode {
private:
  Grid *grid;
  DisplayWriter *displayWriter;
  Control *control;
  ProgramController *pc;
  int activeRow;
  int field;

public:
  NoteMode(Grid *grid, DisplayWriter *displayWriter, Control *control,
	   ProgramController *pc) {
    this->grid = grid;
    this->displayWriter = displayWriter;
    this->control = control;
    this->pc = pc;
  }

  void start() {
    field = 0;
    activeRow = -1;
    displayWriter->clear()->string("NOTE          >")->cr();
    displayWriter->string("  <select row>")->cr();
  }

  void stop() {
    pc->highlightRow = -1;
    pc->draw();
  }

  bool tick() {
    bool change = false;
    char row, column;
    int mod;
    int value;

    while (grid->getPress(&row, &column))
      change = true;

    if (change) {
      pc->highlightRow = row;
      pc->draw();
      this->activeRow = row;
    }

    if (this->activeRow < 0)
      return true;

    if (control->getSelect()) {
      if (++field > 1)
	field = 0;
      change = true;
    }

    mod = control->getMod();

    if (!change && !mod)
      return true;

    displayWriter->clear();
    switch (field) {
    case 0:
      value = pc->current->note[this->activeRow];
      value = MIN(127, MAX(0, value + mod));
      pc->current->note[this->activeRow] = value;
      displayWriter->string("NOTE          >")->cr()->		\
        string("  ")->integer(this->activeRow)->string(": ")->	\
	note(value)->cr();
      break;
    case 1:
      value = pc->current->velocity[this->activeRow];
      value = MIN(127, MAX(0, value + mod));
      pc->current->velocity[this->activeRow] = value;
      displayWriter->string("VELOCITY      >")->cr()->		\
        string("  ")->integer(this->activeRow)->string(": ")->	\
	integer(value)->cr();
      break;
    }

    return true;
  }
};

#endif
