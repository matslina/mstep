#ifndef MODE_HPP_49a8dfuas48fjasefjaijf
#define MODE_HPP_49a8dfuas48fjasefjaijf


class Mode {
public:
  virtual void start() {}
  virtual void stop() {}
  virtual bool tick() = 0;
};

class MultiFieldMode : public Mode {
private:
  char i;

protected:
  ProgramController *patternController;
  DisplayWriter *displayWriter;
  Control *control;
  char numFields;
  const char **fieldName;
  char (ProgramController::**fieldFun)(char);

public:
  void start() {
    i = 0;
    displayWriter->clear()->namedInteger(fieldName[i], (patternController->*fieldFun[i])(0));
  }

  bool tick() {
    bool updateDisplay = false;

    if (control->getSelect()) {
      if (++i >= numFields)
	i = 0;
      updateDisplay = true;
    }

    int mod = control->getMod();
    if (mod || updateDisplay)
      displayWriter->clear()->namedInteger(fieldName[i], (patternController->*fieldFun[i])(mod));

    return true;
  }
};

#endif
