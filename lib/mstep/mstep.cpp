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

  gridWidth = grid->getWidth();
  gridHeight = grid->getHeight();

  numPads = gridWidth * gridHeight;
  gridStateSize = ((numPads / 8) +
		   (numPads % 8 ? 1 : 0));

  buf = (char *)malloc(gridStateSize +
		       gridStateSize +
		       gridStateSize +
		       gridHeight);
  gridState   = buf;
  gridOverlay = buf + gridStateSize;
  gridBuf     = buf + gridStateSize * 2;
  activeNotes = buf + gridStateSize * 3;
  for (int i = 0; i < gridStateSize; i++) gridState[i] = 0;
  for (int i = 0; i < gridStateSize; i++) gridOverlay[i] = 0;
  for (int i = 0; i < gridStateSize; i++) gridBuf[i] = 0;
  for (int i = 0; i < gridHeight; i++) activeNotes[i] = -1;
}

void MStep::run() {
  char row, column, pad;
  char playColumn = -1;
  unsigned long playNext;

  // displayStartupSequence();
  draw();

  while (!control->eventShutdown()) {

    while (grid->eventPress(&row, &column)) {
      pad = row * gridWidth + column;
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
