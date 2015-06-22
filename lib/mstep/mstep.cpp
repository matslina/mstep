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
  gridStateSize = (((gridWidth * gridHeight) / 8) +
		   ((gridWidth * gridHeight) % 8 ? 1 : 0));

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
  activeRow = -1;
  activeColumn = -1;
}

static void displayInteger(Display *display, char *name, int value) {
  char buf[20];
  sprintf(buf, "  %d", value);
  display->clear();
  display->write(0, name);
  display->write(1, buf);
}

void MStep::noteStart() {
  activeRow = -1;
  display->clear();
  display->write(0, F("NOTE"));
  display->write(1, F("  <select row>"));
}

void MStep::noteStop() {
  if (activeRow > -1) {
    overlayHline(activeRow);
    draw();
  }
  activeRow = -1;
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
    rowChanged = true;
  }

  if (rowChanged) {
    if (this->activeRow > -1)
      overlayHline(this->activeRow);
    overlayHline(row);
    draw();
    this->activeRow = row;
  }

  if (this->activeRow < 0)
    return;

  value = pattern[activePattern].note[this->activeRow];

  mod = control->getUp() - control->getDown();
  if (mod) {
    value = MIN(127, MAX(0, value + mod));
    noteChanged = true;
  }

  if (rowChanged || noteChanged) {
    sprintf(buf, F("  %d: %s%d"),
	    this->activeRow, notes[value % 12], value / 12 - 1);
    display->clear();
    display->write(0, F("NOTE"));
    display->write(1, buf);
  }

  pattern[activePattern].note[this->activeRow] = value;
}

void MStep::tempoStart() {
  displayInteger(display, F("TEMPO"), tempo);
}

void MStep::tempoStop() {
  // TODO: remove if redundant
}

void MStep::tempoTick() {
  int mod;

  mod = control->getUp() - control->getDown();
  if (!mod)
    return;

  tempo = MIN(240, MAX(1, tempo + mod));
  displayInteger(display, F("TEMPO"), tempo);
}

void MStep::patternStart() {
  displayInteger(display, F("PATTERN"), activePattern);
}

void MStep::patternStop() {
  // TODO: remove if redundant
}

void MStep::patternTick() {
  int mod;

  mod = control->getUp() - control->getDown();
  if (!mod)
    return;

  activePattern = MIN(gridHeight - 1, MAX(0, activePattern + mod));
  displayInteger(display, F("PATTERN"), activePattern);
}

void MStep::playStart() {
  // schedule next step to happen immediately
  playNext = time();

  // pretend we just played the final column so that the next
  // playTick() progresses to the first column to start playback. note
  // that we overlay a hline but never call draw(), so playTick() will
  // clear it in the next invocation and the user only sees a column
  // light up in row 0. A bit of a kludge... =/
  activeColumn = gridWidth - 1;
  overlayVline(activeColumn);
}

void MStep::playStop() {
  // send note off for notes currently on
  for (int i = 0; i < gridHeight; i ++) {
    if (pattern[activePattern].active[i] >= 0) {
      midi->noteOn(0, pattern[activePattern].active[i], 0);
      pattern[activePattern].active[i] = -1;
    }
  }

  // and clear the grid
  overlayVline(activeColumn);
  activeColumn = -1;
  draw();
}

int MStep::playTick() {
  int pad;
  char *grid = pattern[activePattern].grid;
  char *note = pattern[activePattern].note;
  char *velocity = pattern[activePattern].velocity;
  char *active = pattern[activePattern].active;
  unsigned long int now;

  now = time();
  if (playNext > now)
    return playNext - now;

  // step to next column and update grid
  overlayVline(activeColumn);
  activeColumn = (activeColumn + 1) % gridWidth;
  overlayVline(activeColumn);
  draw();

  for (int i = 0; i < gridHeight; i ++) {
    // note off for currently playing notes
    if (active[i] >= 0) {
      midi->noteOn(9, active[i], 0); // FIXME: don't hardcode channel
      active[i] = -1;
    }

    // note on according to the grid
    pad = i * gridWidth + activeColumn;
    if (grid[pad / 8] & (1 << (pad % 8))) {
      midi->noteOn(9, note[i], velocity[i]); // FIXME: don't hardcode channel
      active[i] = note[i];
    }
  }

  // schedule next step
  playNext += (240000 / tempo) / gridWidth;

  return MAX(0, playNext - time());
}


void MStep::run() {
  char row, column;
  int pad;
  int event;
  int mode;
  int sleepDuration;

  // displayStartupSequence();
  mode = 0;
  control->indicate(mode);
  draw();

  while (1) {
    sleepDuration = 30;

    event = control->getEvent();

    // special treatment of quit and play events, coz they're special.
    if (event & Control::QUIT)
      break;
    if (event & Control::PLAY) {
      if (mode & Control::PLAY)
	playStop();
      else
	playStart();
      mode ^= event & Control::PLAY;
      control->indicate(mode);
    }

    // only process event if it's unambiguous and it isn't garbage
    if (event && !(event & (event - 1)) &&
	event & (Control::NOTE | Control::TEMPO | Control::PATTERN)) {

      // with the exception of PLAY, all modes are mutually exclusive,
      // so we stop the current mode
      switch (mode & ~Control::PLAY) {
      case Control::NOTE:
	noteStop();
	break;
      case Control::TEMPO:
	tempoStop();
	break;
      case Control::PATTERN:
	patternStop();
	break;
      }

      // start the requested mode, unless it was just stopped in which
      // case we bring back the default display
      switch (event & ~mode) {
      case Control::NOTE:
	noteStart();
	break;
      case Control::TEMPO:
	tempoStart();
	break;
      case Control::PATTERN:
	patternStart();
	break;
      default:
	display->clear();
	break;
      }

      mode = (mode & Control::PLAY) | (event & ~mode);
      control->indicate(mode);
    }

    // unless in note mode, grid press updates grid state
    if (!(mode & Control::NOTE)) {
      while (grid->getPress(&row, &column)) {
	pad = row * gridWidth + column;
	pattern[activePattern].grid[pad >> 3] ^= 1 << (pad & 0x7);
      }
      draw();
    }

    // tick() according to mode
    switch (mode & ~Control::PLAY) {
    case Control::NOTE:
      noteTick();
      break;
    case Control::TEMPO:
      tempoTick();
      break;
    case Control::PATTERN:
      patternTick();
      break;
    }
    if (mode & Control::PLAY)
      sleepDuration = MIN(sleepDuration, playTick());

    sleep(sleepDuration);
  }
}

void MStep::draw() {
  for (int i = 0; i < gridStateSize; i++)
    gridBuf[i] = pattern[activePattern].grid[i] ^ gridOverlay[i];
  grid->draw(gridBuf);
}

void MStep::overlayVline(char column) {
  for (int i = column; i < gridWidth * gridHeight; i += gridWidth)
    gridOverlay[i >> 3] ^= 1 << (i & 7);
}

void MStep::overlayHline(char row) {
  for (int i = row * gridWidth; i < (row + 1) * gridWidth; i++)
    gridOverlay[i >> 3] ^= 1 << (i & 7);
}

void MStep::displayStartupSequence() {
}
