#ifndef PATTERNMODE_HPP_98387245kjdrkjfdsheiudsjdskj
#define PATTERNMODE_HPP_98387245kjdrkjfdsheiudsjdskj

#include "mstep.hpp"
#include "mode.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"

class PatternMode : Mode {
public:
  int field;
  DisplayWriter *displayWriter;
  Control *control;
  PatternController *pc;

  PatternMode(DisplayWriter *displayWriter, Control *control, PatternController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->pc = pc;
  }

  void start() {
    field = 0;
    displayWriter->clear()->namedInteger("PATTERN       >", pc->currentIndex);
  }

  void stop() {
  }

  unsigned int tick() {
    int mod;
    bool displayAnyway = false;

    if (control->getSelect()) {
      if (++field > 2)
	field = 0;
      displayAnyway = true;
    }

    mod = control->getMod();
    if (!mod && !displayAnyway)
      return 100;

    displayWriter->clear();
    switch (field) {
    case 0:
      pc->change(mod);
      displayWriter->namedInteger("PATTERN       >", pc->currentIndex);
      break;
    case 1:
      pc->current->swing = MIN(75, MAX(50, pc->current->swing + mod));
      displayWriter->namedInteger("SWING         >", pc->current->swing);
      break;
    case 2:
      pc->current->channel = MIN(16, MAX(1, pc->current->channel + mod));
      displayWriter->namedInteger("CHANNEL       >", pc->current->channel);
      break;
    }

    return 100;
  }

};

#endif
