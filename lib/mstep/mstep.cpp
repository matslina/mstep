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

  // this is a bit unwieldy, but helps track memory consumption
  buf = (char *)malloc(gridStateSize + // gridOverlay
		       gridStateSize + // gridBuf
		       gridHeight * sizeof(pattern_t) + // patterns
		       gridHeight * (gridStateSize + // pattern grids
				     gridHeight + // pattern notes
				     gridHeight + // pattern velocities
				     gridHeight)); // pattern active notes

  // assign pointers
  gridOverlay = buf;
  gridBuf     = buf + gridStateSize;
  pattern = (pattern_t *)(buf + gridStateSize * 2);
  char *tmpp = buf + gridStateSize * 2 + gridHeight * sizeof(pattern_t);
  for (int i = 0; i < gridHeight; i++) {
    pattern[i].grid = tmpp;
    pattern[i].note = tmpp + gridStateSize;
    pattern[i].velocity = tmpp + gridStateSize + gridHeight;
    pattern[i].active = tmpp + gridStateSize + gridHeight * 2;
    tmpp += gridStateSize + gridHeight * 3;
  }

  // initialize. this should load from eeprom or use a default.
  for (int i = 0; i < gridStateSize; i++) gridOverlay[i] = 0;
  for (int i = 0; i < gridStateSize; i++) gridBuf[i] = 0;
  for (int i = 0; i < gridHeight; i++) {
    for (int j = 0; j < gridStateSize; j++) pattern[i].grid[j] = 0;
    for (int j = 0; j < gridHeight; j++) pattern[i].note[j] = 36 + j;
    for (int j = 0; j < gridHeight; j++) pattern[i].velocity[j] = 127;
    for (int j = 0; j < gridHeight; j++) pattern[i].active[j] = -1;
  }

  activePattern = 0;
}

void MStep::run() {
  char row, column;
  char playColumn = -1;
  unsigned long playNext;
  int pad;

  // displayStartupSequence();
  draw();

  while (!control->eventShutdown()) {

    while (grid->eventPress(&row, &column)) {
      pad = row * gridWidth + column;
      pattern[activePattern].grid[pad >> 3] ^= 1 << (pad & 0x7);
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
	for (int i = 0; i < gridHeight; i ++) {
	  if (pattern[activePattern].active[i] >= 0) {
	    midi->noteOn(0, pattern[activePattern].active[i], 0);
	    pattern[activePattern].active[i] = -1;
	  }
	}
      }
    }

    if (playColumn >= 0 && playNext <= time()) {
      playColumn = (playColumn + 1) % gridWidth;
      play(playColumn);
      playNext += 125;
      overlayHline(playColumn);
      draw();
    }

    sleep(30);
  }
}

void MStep::play(char column) {
  int pad;
  char *grid = pattern[activePattern].grid;
  char *note = pattern[activePattern].note;
  char *velocity = pattern[activePattern].velocity;
  char *active = pattern[activePattern].active;

  for (int i = 0; i < gridHeight; i ++) {
    if (active[i] >= 0) {
      midi->noteOn(9, active[i], 0);
      active[i] = -1;
    }

    pad = i * gridWidth + column;
    if (grid[pad / 8] & (1 << (pad % 8))) {
      midi->noteOn(9, note[i], velocity[i]);
      active[i] = 36 + i;
    }
  }
}

void MStep::draw() {
  for (int i = 0; i < gridStateSize; i++)
    gridBuf[i] = pattern[activePattern].grid[i] ^ gridOverlay[i];
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
}
