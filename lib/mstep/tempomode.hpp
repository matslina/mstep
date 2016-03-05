#ifndef TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml
#define TEMPOMODE_HPP_9439043982982ksfdfdskfdkfdkmfdskml

#include "mode.hpp"
#include "mstep.hpp"
#include "displaywriter.hpp"

class TempoMode : public Mode {
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

  bool tick() {
    int mod;

    mod = control->getMod();
    if (!mod)
      return true;

    *tempo = MIN(240, MAX(1, *tempo + mod));
    displayWriter->clear()->namedInteger("TEMPO", *tempo);

    return true;
  }
};

#endif
