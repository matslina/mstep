#ifndef TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml
#define TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"
#include "patterncontroller.hpp"

class TempoMode : public Mode {
private:
  DisplayWriter *displayWriter;
  Control *control;
  PatternController *pc;

public:
  TempoMode(DisplayWriter *displayWriter, Control *control, PatternController *pc) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->pc = pc;
  }

  void start() {
    displayWriter->clear()->namedInteger("TEMPO", pc->program.tempo);
  }

  bool tick() {
    int mod;

    mod = control->getMod();
    if (!mod)
      return true;

    pc->program.tempo = MIN(240, MAX(1, pc->program.tempo + mod));
    displayWriter->clear()->namedInteger("TEMPO", pc->program.tempo);

    return true;
  }
};

#endif
