#ifndef LOADMODE_HPP_ddkritigdksiefdk97978736152849dkrk
#define LOADMODE_HPP_ddkritigdksiefdk97978736152849dkrk

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"
#include "storagecontroller.hpp"

class LoadMode : public Mode {
public:
  DisplayWriter *displayWriter;
  Control *control;
  StorageController *storage;
  PatternController *pc;
  int slot;

  LoadMode(DisplayWriter *displayWriter, Control *control,
	   StorageController *storage, PatternController *pc) {
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

  void stop() {
  }

  unsigned int tick() {
    int mod;

    if (storage->numEntries <= 0)
      return 0;

    if (control->getSelect() && slot >= 0) {
      storage->loadProgram(slot, &pc->program);
      pc->draw();
      displayWriter->clear()->string("SLOT ")->integer(slot)->	\
	string(" LOADED")->cr();

      return 1231;
    }

    mod = control->getMod();
    if (!mod)
      return 123123;

    slot = MAX(0, MIN(storage->numEntries - 1, slot + mod));
    displayWriter->clear()->namedInteger("LOAD SLOT", slot);
  }
};

#endif
