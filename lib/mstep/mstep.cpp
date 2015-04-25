#include "mstep.hpp"

#if 0
class Grid {
 public:
  virtual char *getEvents() = 0;
  virtual void clearEvents() = 0;
  virtual void draw(char *grid) = 0;
  virtual char getWidth() = 0;
  virtual char getHeight() = 0;
};



class Control {
 public:
  virtual ~Control() {}
  virtual bool getButtonEvents() = 0;
  virtual void clearButtonEvents() = 0;
  virtual bool shutdownRequested() = 0;
};

class Display {
 public:
  virtual void write() = 0;
};

#endif

MStep::MStep(Grid *grid, Control *control, Display *display,
	     void (*sleep)(unsigned long)) {
  this->grid = grid;
  this->control = control;
  this->display = display;
  this->sleep = sleep;
}

void MStep::run() {
  char d[12];
  int i = 0;
  while (!control->shutdownRequested()) {
    grid->draw(d);
    sleep(1031);
    d[i % 12] = 0x00;
    i++;
    d[i % 12] = i & 0xff;
  }
}
