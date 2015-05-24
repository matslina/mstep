#include <stdlib.h>

#include <stdio.h>
#include "mstep.hpp"

#ifndef F
#define F(x) (char *)x
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

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
  tempo = DEFAULT_TEMPO;
  stepDelay = (240000 / tempo) / gridWidth;
}

void MStep::noteTick() {
  bool rowChanged = false;
  bool noteChanged = false;
  char buf[20];
  char row, column;
  int mod;
  int value;
  const char *notes[] = {"C", "C#", "D", "D#", "E", "F",
			 "F#", "G", "G#", "A", "A#", "B"};

  while (grid->getPress(&row, &column)) {
    this->noteRow = row;
    rowChanged = true;
  }

  if (this->noteRow < 0)
    return;

  value = pattern[activePattern].note[this->noteRow];

  mod = control->getUp() - control->getDown();
  if (mod) {
    value = MIN(127, MAX(0, value + mod));
    noteChanged = true;
  }

  if (rowChanged || noteChanged) {
    sprintf(buf, F("  %d: %s%d"),
	    this->noteRow, notes[value % 12], value / 12 - 1);
    display->clear();
    display->write(0, F("NOTE"));
    display->write(1, buf);
  }

  pattern[activePattern].note[this->noteRow] = value;
}

void MStep::tempoTick() {
  int mod;
  char buf[10];

  mod = control->getUp() - control->getDown();
  if (!mod)
    return;

  tempo = MIN(240, MAX(1, tempo + mod));
  stepDelay = (240000 / tempo) / gridWidth;
  sprintf(buf, "  %d BPM", this->tempo);
  display->clear();
  display->write(0, F("TEMPO"));
  display->write(1, buf);
}

void MStep::run() {
  char row, column;
  char playColumn = -1;
  unsigned long playNext;
  int pad;
  int event;
  bool noteActive = false;
  bool tempoActive = false;
  int noteRow;
  int noteValuePrev;
  int noteValue;
  char buf[10];

  // displayStartupSequence();
  draw();

  while (1) {
    event = control->getEvent();

    if (event & Control::QUIT)
      break;

    if (event & Control::NOTE && not tempoActive) {
      if (noteActive) {
	control->indicate(playColumn >= 0 ? Control::PLAY : 0);
	noteActive = false;
	display->clear();
      } else {
	control->indicate(Control::NOTE | (playColumn >= 0 ? Control::PLAY : 0));
	noteActive = true;
	this->noteRow = -1;
	display->write(0, F("NOTE"));
	display->write(1, F("  select row"));
      }
    }

    if (event & Control::TEMPO && not noteActive) {
      if (tempoActive) {
	control->indicate(playColumn >= 0 ? Control::PLAY : 0);
	tempoActive = false;
	display->clear();
      } else {
	control->indicate(Control::TEMPO | (playColumn >= 0 ? Control::PLAY : 0));
	tempoActive = true;
	display->clear();
	sprintf(buf, "  %d BPM", tempo);
	display->write(0, F("TEMPO"));
	display->write(1, buf);
      }
    }

    if (noteActive)
      noteTick();

    if (tempoActive)
      tempoTick();

    if (!noteActive && !tempoActive) {
      while (grid->getPress(&row, &column)) {
	pad = row * gridWidth + column;
	pattern[activePattern].grid[pad >> 3] ^= 1 << (pad & 0x7);
	draw();
      }
    }

    if (event & Control::PLAY) {
      if (playColumn < 0) {
	playColumn = gridWidth - 1;
	playNext = time();
	control->indicate(Control::PLAY);
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
	control->indicate(0);
      }
    }

    if (playColumn >= 0 && playNext <= time()) {
      playColumn = (playColumn + 1) % gridWidth;
      play(playColumn);
      playNext += stepDelay;
      overlayClear();
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
      active[i] = note[i];
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
  for (int i = column; i < numPads; i += gridWidth)
    gridOverlay[i >> 3] ^= 1 << (i & 7);
}

void MStep::displayStartupSequence() {
}
