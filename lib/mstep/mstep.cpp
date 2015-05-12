#include <stdlib.h>
#include "mstep.hpp"

MStep::MStep(Grid *grid, Control *control, Display *display, MIDI *midi,
	     void (*sleep)(unsigned long),
	     unsigned long (*time)(void)) {
  this->grid = grid;
  this->control = control;
  this->display = display;
  this->midi = midi;
  this->sleep = sleep;
  this->time = time;
  this->gridWidth = grid->getWidth();
  this->gridHeight = grid->getHeight();
  this->numPads = gridWidth * gridHeight;
  this->gridStateSize = (((gridWidth * gridHeight) / 8) +
			 ((gridWidth * gridHeight) % 8 ? 1 : 0));
  this->gridState = (char *)malloc(gridStateSize * 3);
  this->gridOverlay = gridState + gridStateSize;
  this->gridBuf = gridState + gridStateSize * 2;
  this->activeNotes = (char *)malloc(gridHeight);
  for (int i = 0; i < gridStateSize * 3; i++)
    gridState[i] = 0;
  for (int i = 0; i < gridHeight; i++)
    activeNotes[i] = -1;
}

void MStep::run() {
  char r, c, pad;
  char playColumn = -1;
  unsigned long playNext;

  // displayStartupSequence();
  draw();

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
      play(playColumn);
      playNext += 500;
      overlayHline(playColumn);
      draw();
    }

    sleep(30);
  }
}

void MStep::play(char column) {
  int pad;

  for (int i = 0; i < gridHeight; i ++) {
    if (activeNotes[i] >= 0) {
      midi->noteOn(1, activeNotes[i], 0);
      activeNotes[i] = -1;
    }

    pad = i * gridWidth + column;
    if (gridState[pad / 8] & (1 << (pad % 8))) {
      midi->noteOn(1, 60 + i, 127);
      activeNotes[i] = 60 + i;
    }
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
