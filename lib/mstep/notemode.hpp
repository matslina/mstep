#ifndef NOTEMODE_HPP_94lkdvnmxcsvslkfa9jseuixckmsd
#define NOTEMODE_HPP_94lkdvnmxcsvslkfa9jseuixckmsd

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"

class NoteMode : Mode {
public:
  Grid *grid;
  DisplayWriter *displayWriter;
  Control *control;
  PatternController *pc;
  int activeRow;

  NoteMode(Grid *grid, DisplayWriter *displayWriter, Control *control,
	   PatternController *pc) {
    this->grid = grid;
    this->displayWriter = displayWriter;
    this->control = control;
    this->pc = pc;
  }

  void start() {
    activeRow = -1;
    displayWriter->clear()->string("NOTE")->cr();
    displayWriter->string("  <select row>")->cr();
  }

  void stop() {
    pc->highlightRow = -1;
    pc->draw();
  }

  unsigned int tick() {
    bool rowChanged = false;
    bool noteChanged = false;
    char row, column;
    int mod;
    int value;

    while (grid->getPress(&row, &column))
      rowChanged = true;

    if (rowChanged) {
      pc->highlightRow = row;
      pc->draw();
      this->activeRow = row;
    }

    if (this->activeRow < 0)
      return 100;

    value = pc->current->note[this->activeRow];

    mod = control->getMod();
    if (mod) {
      value = MIN(127, MAX(0, value + mod));
      noteChanged = true;
    }

    if (rowChanged || noteChanged) {
      displayWriter->clear()->string("NOTE")->cr()->		\
        string("  ")->integer(this->activeRow)->string(": ")->	\
	note(value)->cr();
    }

    pc->current->note[this->activeRow] = value;

    return 100;
  }
};

#endif
