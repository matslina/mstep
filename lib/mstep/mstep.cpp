#include <stdlib.h>

#include "mstep.hpp"
#include "stuff.hpp"

#define GRID_W MSTEP_GRID_WIDTH
#define GRID_H MSTEP_GRID_HEIGHT

#ifndef F
#define F(x) (char *)x
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

class Mode {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual unsigned int tick() = 0;
private:
};


MStep::MStep(Grid *grid, Control *control, Display *display, MIDI *midi,
	     void (*sleep)(unsigned long),
	     unsigned long (*time)(void)) {
  this->grid = grid;
  this->control = control;
  this->display = display;
  this->midi = midi;
  this->sleep = sleep;
  this->time = time;
}

void MStep::run() {
  Sequencer s = Sequencer(grid, control, display, midi, sleep, time);
  s.run();
}


Sequencer::Sequencer(Grid *grid, Control *control, Display *display, MIDI *midi,
		     void (*sleep)(unsigned long),
		     unsigned long (*time)(void)) {
  this->grid = grid;
  this->control = control;
  this->display = display;
  this->midi = midi;
  this->sleep = sleep;
  this->time = time;

  // pattern defaults
  for (int i = 0; i < GRID_H; i++) {
    pattern[i].channel = DEFAULT_CHANNEL;
    pattern[i].column = -1;
    pattern[i].swing = 50;
    for (int j = 0; j < GRID_BYTES; j++)
      pattern[i].grid[j] = 0;
    for (int j = 0; j < GRID_H; j++) {
      pattern[i].note[j] = 36 + j;
      pattern[i].velocity[j] = 127;
      pattern[i].active[j] = -1;
    }

    // hard coded volca notes as defaults
    // hack hack hack hack hack
    pattern[i].note[0] = 36;
    pattern[i].note[1] = 38;
    pattern[i].note[2] = 43;
    pattern[i].note[3] = 50;
    pattern[i].note[4] = 42;
    pattern[i].note[5] = 46;
    pattern[i].note[6] = 39;
  }

  activePattern = 0;
  tempo = DEFAULT_TEMPO;
  activeRow = -1;
}

static int appends(char *buf, char *s) {
  int i = 0;

  while (*s) {
    buf[i++] = *s;
    s++;
  }
  buf[i] = '\0';

  return i;
}

static int appendi(char *buf, int v) {
  int i = 0;
  int n = v;

  if (v > 99) {
    buf[i++] = n / 100 + '0';
    n %= 100;
  }
  if (v > 9) {
    buf[i++] = n / 10 + '0';
    n %= 10;
  }
  buf[i++] = n + '0';
  buf[i] = '\0';

  return i;
}

static void displayInteger(Display *display, char *name, int value) {
  char buf[16];
  int i = 0;
  display->clear();
  display->write(0, name);
  i += appends(buf, "  ");
  appendi(buf + i, value);
  display->write(1, buf);
}


class PatternMode : Mode {
public:
  int field;
  Display *display;
  Control *control;
  pattern_t *pattern;
  int npattern;
  int *active;

  PatternMode(Display *display, Control *control, pattern_t *pattern,
	      int npattern, int *active) {
    this->display = display;
    this->control = control;
    this->pattern = pattern;
    this->npattern = npattern;
    this->active = active;
  }

  void start() {
    field = 0;
    displayInteger(display, "PATTERN       >", *active);
  }

  void stop() {
  }

  unsigned int tick() {
    int mod;
    bool displayAnyway = false;

    if (control->getSelect()) {
      if (++field > 2)
	field = 0;
      displayAnyway = true;
    }

    mod = control->getMod();
    if (!mod && !displayAnyway)
      return 100;

    switch (field) {
    case 0:
      *active = MIN(npattern - 1, MAX(0, *active + mod));
      displayInteger(display, F("PATTERN"), *active);
      // FIXME: grid has to be redrawn here
      break;
    case 1:
      pattern[*active].swing = MIN(75, MAX(50, pattern[*active].swing + mod));
      displayInteger(display, F("SWING"), pattern[*active].swing);
      break;
    case 2:
      pattern[*active].channel = MIN(16, MAX(1, pattern[*active].channel + mod));
      displayInteger(display, F("CHANNEL"), pattern[*active].channel);
      break;
    }

    return 100;
  }

};



void Sequencer::noteStart() {
  activeRow = -1;
  display->clear();
  display->write(0, F("NOTE"));
  display->write(1, F("  <select row>"));
}

void Sequencer::noteStop() {
  if (activeRow > -1) {
    overlayHline(activeRow);
    draw();
  }
  activeRow = -1;
}

void Sequencer::noteTick() {
  bool rowChanged = false;
  bool noteChanged = false;
  char buf[16];
  char row, column;
  int mod;
  int value;
  int i;
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

  mod = control->getMod();
  if (mod) {
    value = MIN(127, MAX(0, value + mod));
    noteChanged = true;
  }

  if (rowChanged || noteChanged) {
    display->clear();
    display->write(0, F("NOTE"));
    i = appends(buf, F("  "));
    i += appendi(buf + i, this->activeRow);
    i += appends(buf + i, F(": "));
    i += appends(buf + i, (char *)notes[value % 12]);
    i += appendi(buf + i, value / 12 - 1);
    i += appends(buf + i, F(" ("));
    i += appendi(buf + i, value);
    appends(buf + i, F(")"));
    display->write(1, buf);
  }

  pattern[activePattern].note[this->activeRow] = value;
}

class TempoMode : Mode {
public:
  Display *display;
  Control *control;
  int *tempo;

  TempoMode(Display *display, Control *control, int *tempo) {
    this->display = display;
    this->control = control;
    this->tempo = tempo;
  }

  void start() {
    displayInteger(display, F("TEMPO"), *tempo);
  }

  void stop() {
  }

  unsigned int tick() {
    int mod;

    mod = control->getMod();
    if (!mod)
      return 123123;

    *tempo = MIN(240, MAX(1, *tempo + mod));
    displayInteger(display, F("TEMPO"), *tempo);
  }
};

void Sequencer::playStart() {
  playPattern = activePattern;

  // schedule next step to happen immediately
  playNext = time();
  pattern[playPattern].swingDelay = 0;

  // pretend we just played the final column so that the next
  // playTick() progresses to the first column to start playback. note
  // that we overlay a hline but never call draw(), so playTick() will
  // clear it in the next invocation and the user only sees a column
  // light up in row 0. A bit of a kludge... =/
  pattern[playPattern].column = GRID_W - 1;
  overlayVline(pattern[playPattern].column);
}

void Sequencer::playStop() {
  // send note off for notes currently on
  for (int i = 0; i < GRID_H; i ++) {
    if (pattern[playPattern].active[i] >= 0) {
      midi->noteOn(pattern[playPattern].channel,
		   pattern[playPattern].active[i], 0);
      pattern[playPattern].active[i] = -1;
    }
  }

  // and clear the grid
  overlayVline(pattern[playPattern].column);
  pattern[playPattern].column = -1;
  draw();
}

int Sequencer::playTick() {
  int pad;
  pattern_t *p;
  unsigned long int now;
  unsigned long int when;

  p = &pattern[playPattern];

  now = time();
  when = playNext + p->swingDelay;
  if (when > now)
    return when - now;

  // note off for currently playing notes
  for (int i = 0; i < GRID_H; i ++) {
    if (p->active[i] >= 0) {
      midi->noteOn(p->activeChannel, p->active[i], 0);
      p->active[i] = -1;
    }
  }

  // step one column forward. currently played pattern may not be the
  // one currently displayed (activePattern), so take care when
  // drawing those columns. when wrapping around we always start
  // playing the displayed pattern.
  overlayVline(p->column);
  if (++p->column == GRID_W) {
    if (playPattern != activePattern) {
      p->column = -1;
      p = &pattern[activePattern];
    }
    playPattern = activePattern;
    p->column = 0;
  }
  overlayVline(p->column);
  draw();

  // note on according to the grid
  for (int i = 0; i < GRID_H; i ++) {
    pad = i * GRID_W + p->column;
    if (p->grid[pad / 8] & (1 << (pad % 8))) {
      midi->noteOn(p->channel, p->note[i], p->velocity[i]);
      p->active[i] = p->note[i];
    }
  }
  p->activeChannel = p->channel;

  // schedule next step
  playNext += (240000 / tempo) / GRID_W;
  p->swingDelay = 0;
  if (!(p->column & 1))
    p->swingDelay = ((float)(p->swing - 50) / 50) * (240000 / tempo) / GRID_W;

  return MAX(0, playNext + p->swingDelay - time());
}


void Sequencer::run() {
  char row, column;
  int pad;
  int event;
  int mode;
  int sleepDuration;

  PatternMode pmode = PatternMode(display, control, pattern,
				  GRID_H, &activePattern);
  TempoMode tmode = TempoMode(display, control, &tempo);

  display->write(0, F("initializing"));
  displayStartupSequence();
  mode = 0;
  control->indicate(mode);
  draw();
  while (grid->getPress(&row, &column));
  display->clear();
  display->write(0, F("MStep 4711"));
  display->write(1, F("  ready"));

  while (1) {

    // playback is prio 1. don't bother processing events unless we
    // have "enough" time.
    if (mode & Control::PLAY) {
      sleepDuration = playTick();
      if (sleepDuration < 30) {
	sleep(sleepDuration);
	continue;
      }
    }
    sleep(15);

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
	event & (Control::NOTE | Control::TEMPO |
		 Control::PATTERN)) {

      // with the exception of PLAY, all modes are mutually exclusive,
      // so we stop the current mode
      switch (mode & ~Control::PLAY) {
      case Control::NOTE:
	noteStop();
	break;
      }

      // start the requested mode, unless it was just stopped in which
      // case we bring back the default display
      switch (event & ~mode) {
      case Control::NOTE:
	noteStart();
	break;
      case Control::TEMPO:
	tmode.start();
	break;
      case Control::PATTERN:
	pmode.start();
	break;
      default:
	display->clear();
	break;
      }

      mode = (mode & Control::PLAY) | (event & ~mode);
      control->indicate(mode);
    }

    if (event & Control::COPY) {
      for (int i = 0; i < GRID_BYTES; i++) {
	clipboard.grid[i] = pattern[activePattern].grid[i];
      }
    } else if (event & Control::PASTE) {
      for (int i = 0; i < GRID_BYTES; i++)
	pattern[activePattern].grid[i] |= clipboard.grid[i];
      draw();
    } else if (event & Control::CLEAR) {
      for (int i = 0; i < GRID_BYTES; i++)
	pattern[activePattern].grid[i] = 0;
      draw();
    }

    // unless in note mode, grid press updates grid state
    if (!(mode & Control::NOTE)) {
      while (grid->getPress(&row, &column)) {
	pad = row * GRID_W + column;
	pattern[activePattern].grid[pad >> 3] ^= 1 << (pad & 0x7);
	draw();
      }
    }

    // tick() according to mode
    switch (mode & ~Control::PLAY) {
    case Control::NOTE:
      noteTick();
      break;
    case Control::TEMPO:
      tmode.tick();
      break;
    case Control::PATTERN:
      pmode.tick();
      break;
    }
  }
}

void Sequencer::draw() {
  if (activePattern == playPattern) {
    for (int i = 0; i < GRID_BYTES; i++)
      gridBuf[i] = pattern[playPattern].grid[i] ^ gridOverlay[i];
    grid->draw(gridBuf);
  }
  else
    grid->draw(pattern[activePattern].grid);
}

void Sequencer::overlayVline(char column) {
  for (int i = column; i < GRID_W * GRID_H; i += GRID_W)
    gridOverlay[i >> 3] ^= 1 << (i & 7);
}

void Sequencer::overlayHline(char row) {
  for (int i = row * GRID_W; i < (row + 1) * GRID_W; i++)
    gridOverlay[i >> 3] ^= 1 << (i & 7);
}

void Sequencer::displayStartupSequence() {
  for (int i=0; i < GRID_W * 2; i++) {
    overlayVline(i % GRID_W);
    draw();
  }

  for (int i=0; i < GRID_H * 2; i++) {
    overlayHline(i % GRID_H);
    draw();
  }

  for (int i=0; i < GRID_BYTES; i++)
    gridOverlay[i] = 0;
}
