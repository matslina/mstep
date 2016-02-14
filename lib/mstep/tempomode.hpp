#ifndef TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml
#define TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"

class TempoMode : Mode {
public:
  DisplayWriter *displayWriter;
  Control *control;
  int *tempo;

  TempoMode(DisplayWriter *displayWriter, Control *control, int *tempo) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->tempo = tempo;
  }

  void start() {
    displayWriter->clear()->namedInteger("TEMPO", *tempo);
  }

  void stop() {
  }

  unsigned int tick() {
    int mod;

    mod = control->getMod();
    if (!mod)
      return 123123;

    *tempo = MIN(240, MAX(1, *tempo + mod));
    displayWriter->clear()->namedInteger("TEMPO", *tempo);
  }
};

#endif
