#ifndef SAVEMODE_HPP_kgjsmcmcmmnxzcvbnsadc239czc
#define SAVEMODE_HPP_kgjsmcmcmmnxzcvbnsadc239czc

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"
#include "storagecontroller.hpp"

class SaveMode : public Mode {
private:
  DisplayWriter *displayWriter;
  Control *control;
  StorageController *storage;
  PatternController *pc;
  int slot;

public:
  SaveMode(DisplayWriter *displayWriter, Control *control,
	   StorageController *storage, PatternController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->storage = storage;
    this->pc = pc;
  }

  void start() {
    slot = -1;
    displayWriter->clear()->string("SAVE")->cr()->\
      string("  <select slot>")->cr();
  }

  bool tick() {
    int mod;

    if (control->getSelect() && slot >= 0) {
      storage->saveProgram(slot, &pc->program);
      displayWriter->clear()->string("SAVE")->cr()->\
	string("  SLOT ")->integer(slot)->string(" SAVED")->cr();
      return false;
    }

    mod = control->getMod();
    if (!mod)
      return true;

    slot = MIN(storage->numSlots - 1, MIN(storage->numEntries, MAX(0, slot + mod)));
    displayWriter->clear()->string("SAVE")->cr()->string("  ")->integer(slot);
    if (slot == storage->numEntries)
      displayWriter->string(" (new)");
    displayWriter->cr();

    return true;
  }
};

#endif
