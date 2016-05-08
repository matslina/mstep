#pragma once

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"
#include "storagecontroller.hpp"

class LoadMode : public Mode {
private:
  DisplayWriter *displayWriter;
  Control *control;
  StorageController *storage;
  ProgramController *pc;
  int slot;

public:
  LoadMode(DisplayWriter *displayWriter, Control *control,
	   StorageController *storage, ProgramController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->storage = storage;
    this->pc = pc;
  }

  void start() {
    slot = -1;
    displayWriter->clear()->string("LOAD")->cr()->\
      string(storage->numEntries > 0 ? "  <select>" : "   <empty>")->cr();
  }

  bool tick() {
    int mod;

    if (storage->numEntries <= 0)
      return true;;

    if (control->getSelect() && slot >= 0) {
      storage->loadProgram(slot, &pc->program);
      pc->draw();
      displayWriter->clear()->string("SLOT ")->integer(slot)->	\
	string(" LOADED")->cr();

      return false;
    }

    mod = control->getMod();
    if (!mod)
      return true;

    slot = MAX(0, MIN(storage->numEntries - 1, slot + mod));
    displayWriter->clear()->namedInteger("LOAD SLOT", slot);

    return true;
  }
};
