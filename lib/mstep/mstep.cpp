#include <stdlib.h>
#include "mstep.hpp"

MStep::MStep(Grid *grid, Control *control, Display *display,
	     void (*sleep)(unsigned long),
	     unsigned long (*time)(void)) {
  this->grid = grid;
  this->control = control;
  this->display = display;
  this->sleep = sleep;
  this->time = time;
  this->gridWidth = grid->getWidth();
  this->gridHeight = grid->getHeight();
  this->numPads = gridWidth * gridHeight;
  this->gridStateSize = (((gridWidth * gridHeight) >> 3) +
			 ((gridWidth * gridHeight) & 0x7));
  this->gridState = (char *)malloc(gridStateSize * 2);
  this->gridOverlay = gridState + gridStateSize;
  this->gridBuf = gridState + gridStateSize * 2;
  for (int i = 0; i < gridStateSize * 2; i++)
    gridState[i] = 0;
}

void MStep::run() {
  char r, c, pad;
  char playColumn = -1;
  unsigned long playNext;

  // displayStartupSequence();

  while (!control->eventShutdown()) {

    while (grid->eventPress(&r, &c)) {
      pad = r * gridWidth + c;
      gridState[pad >> 3] ^= 1 << (pad & 0x7);
      draw();
    }

    if (control->eventPlayPause()) {
      if (playColumn < 0) {
	playColumn = gridWidth - 1;
	playNext = time();
      } else {
	playColumn = -1;
	overlayClear();
	draw();
      }
    }

    if (playColumn >= 0 && playNext <= time()) {
      playColumn = (playColumn + 1) % gridWidth;
      playNext += 125;
      overlayHline(playColumn);
      draw();
      // emit midi signals here
    }

    sleep(30);
  }
}

void MStep::draw() {
  for (int i = 0; i < gridStateSize; i++)
    gridBuf[i] = gridState[i] ^ gridOverlay[i];
  grid->draw(gridBuf);
}

void MStep::overlayClear() {
  for (int i = 0; i < gridStateSize; i++)
    gridOverlay[i] = 0;
}

void MStep::overlayHline(char column) {
  for (int i = 0; i < gridStateSize; i++)
    gridOverlay[i] = 0;
  for (int i = column; i < numPads; i += gridWidth)
    gridOverlay[i >> 3] ^= 1 << (i & 7);
}

void MStep::displayStartupSequence() {
  for (int i = 0; i < gridStateSize; i++)
    gridState[i] = 0x55 << (i & 1);
  for (int rounds = 1; rounds < 100; rounds++) {
    grid->draw(gridState);
    for (int i = 0; i < gridStateSize; i++)
      gridState[i] ^= 0xff;
    sleep(500 / rounds);
  }
  for (int i = 0; i < gridStateSize; i++)
    gridState[i] = 0x00;
    grid->draw(gridState);
}
