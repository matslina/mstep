#include "mstep.hpp"

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
