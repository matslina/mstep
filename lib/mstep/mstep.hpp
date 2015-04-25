#ifndef MSTEP_H_dfa982498fjasdjf982093jfas
#define MSTEP_H_dfa982498fjasdjf982093jfas

class Grid {
 public:
  virtual char *getEvents() = 0;
  virtual void clearEvents() = 0;
  virtual void draw(bool *state) = 0;
  virtual char getWidth() = 0;
  virtual char getHeight() = 0;
};



class Control {
 public:
  virtual int getButtonEvents() = 0;
  virtual void clearButtonEvents() = 0;
  virtual bool shutdownRequested() = 0;
};

class Display {
 public:
  virtual void write() = 0;
};

class MStep {
public:
  MStep(Grid *grid, Control *control, Display *display,
	void (*sleep)(unsigned long));
  void run();

private:
  Grid *grid;
  Control *control;
  Display *display;
  void (*sleep)(unsigned long);

};

#endif
