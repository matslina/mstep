#include "mstep.hpp"

MStep::MStep(Grid *grid, Control *control, Display *display,
	     void (*sleep)(unsigned long)) {
  this->grid = grid;
  this->control = control;
  this->display = display;
  this->sleep = sleep;
}

void MStep::run() {
  bool d[128];
  int i = 0;
  while (!control->shutdownRequested()) {
    grid->draw(d);
    sleep(200);
    d[i] = !d[i] ;
    i = (i + 1) % 128;
  }
}
