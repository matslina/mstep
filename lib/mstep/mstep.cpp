#include <stdlib.h>

#include "mstep.hpp"
#include "stuff.hpp"
#include "patterncontroller.hpp"
#include "displaywriter.hpp"


#ifndef F
#define F(x) (char *)x
#endif


class Mode {
public:
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual unsigned int tick() = 0;
private:
};

class Sequencer {
public:
  Sequencer(Grid *grid, Control *control, Display *display, MIDI *midi,
	    void (*sleep)(unsigned long),
	    unsigned long (*time)(void));
  void run();

private:
  Grid *grid;
  Control *control;
  Display *display;
  MIDI *midi;
  pattern_t clipboard;
  pattern_t *playing;
  PatternController *pc;
  DisplayWriter *displayWriter;

  int tempo;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  char activeRow;
  char activePattern;
  unsigned long int playNext;
  void playStart();
  void playStop();
  int playTick();
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


  activePattern = 0;
  tempo = DEFAULT_TEMPO;
  activeRow = -1;
}

class PatternMode : Mode {
public:
  int field;
  DisplayWriter *displayWriter;
  Control *control;
  PatternController *pc;
  int npattern;

  PatternMode(DisplayWriter *displayWriter, Control *control, PatternController *pc,
	      int npattern) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->pc = pc;
    this->npattern = npattern;
  }

  void start() {
    field = 0;
    displayWriter->clear()->namedInteger("PATTERN       >", pc->currentIndex);
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

    displayWriter->clear();
    switch (field) {
    case 0:
      pc->change(mod);
      displayWriter->namedInteger("PATTERN       >", pc->currentIndex);
      // FIXME: grid has to be redrawn here
      break;
    case 1:
      pc->current->swing = MIN(75, MAX(50, pc->current->swing + mod));
      displayWriter->namedInteger("SWING         >", pc->current->swing);
      break;
    case 2:
      pc->current->channel = MIN(16, MAX(1, pc->current->channel + mod));
      displayWriter->namedInteger("CHANNEL       >", pc->current->channel);
      break;
    }

    return 100;
  }

};

class NoteMode : Mode {
public:
  Grid *grid;
  DisplayWriter *displayWriter;
  Control *control;
  PatternController *pc;
  int activeRow;

  NoteMode(Grid *grid, DisplayWriter *displayWriter, Control *control,
	   PatternController *pc) {
    this->grid = grid;
    this->displayWriter = displayWriter;
    this->control = control;
    this->pc = pc;
  }

  void start() {
    activeRow = -1;
    displayWriter->clear()->string("NOTE")->cr();
    displayWriter->string("  <select row>")->cr();
  }

  void stop() {
    pc->highlightRow = -1;
    pc->draw();
  }

  unsigned int tick() {
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
      pc->highlightRow = row;
      pc->draw();
      this->activeRow = row;
    }

    if (this->activeRow < 0)
      return 100;

    value = pc->current->note[this->activeRow];

    mod = control->getMod();
    if (mod) {
      value = MIN(127, MAX(0, value + mod));
      noteChanged = true;
    }

    if (rowChanged || noteChanged) {
      displayWriter->clear()->string("NOTE")->cr()->		\
	string("  ")->integer(activeRow)->string(": ")->	\
	note(value)->cr();
    }

    pc->current->note[this->activeRow] = value;

    return 100;
  }
};

class TempoMode : Mode {
public:
  DisplayWriter *displayWriter;
  Control *control;
  int *tempo;

  TempoMode(DisplayWriter *displayWriter, Control *control, int *tempo) {
    this->displayWriter = displayWriter;
    this->control = control;
    this->tempo = tempo;
  }

  void start() {
    displayWriter->clear()->namedInteger("TEMPO", *tempo);
  }

  void stop() {
  }

  unsigned int tick() {
    int mod;

    mod = control->getMod();
    if (!mod)
      return 123123;

    *tempo = MIN(240, MAX(1, *tempo + mod));
    displayWriter->clear()->namedInteger("TEMPO", *tempo);
  }
};

void Sequencer::playStart() {
  playing = pc->current;
  playNext = time();
  playing->swingDelay = 0;
}

void Sequencer::playStop() {
  // send note off for notes currently on
  for (int i = 0; i < GRID_H; i ++) {
    if (playing->active[i] >= 0) {
      midi->noteOn(playing->channel, playing->active[i], 0);
      playing->active[i] = -1;
    }
  }

  // and clear the grid
  pc->highlightColumn = -1;
  pc->draw();
  playing->column = -1;
}

int Sequencer::playTick() {
  int pad;
  pattern_t *p;
  unsigned long int now;
  unsigned long int when;

  p = playing;

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
  if (++p->column == GRID_W) {
    if (playing != pc->current) {
      p->column = -1;
      playing = pc->current;
      p = pc->current;
    }
    p->column = 0;
  }
  pc->highlightColumn = p->column;
  if (playing != pc->current)
    pc->highlightColumn = -1;
  pc->draw();

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

  DisplayWriter dw = DisplayWriter(display);
  TempoMode tmode = TempoMode(&dw, control, &tempo);
  PatternController ppc = PatternController(grid);
  PatternMode pmode = PatternMode(&dw, control, &ppc, GRID_H);
  NoteMode nmode = NoteMode(grid, &dw, control, &ppc);
  this->pc = &ppc;
  this->displayWriter = &dw;

  displayWriter->clear()->string("initializing")->cr();
  mode = 0;
  control->indicate(mode);
  ppc.draw();
  while (grid->getPress(&row, &column));
  displayWriter->clear()->string("MStep 4711")->cr()->string("  ready")->cr();

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
	nmode.stop();
	break;
      }

      // start the requested mode, unless it was just stopped in which
      // case we bring back the default display
      switch (event & ~mode) {
      case Control::NOTE:
	nmode.start();
	break;
      case Control::TEMPO:
	tmode.start();
	break;
      case Control::PATTERN:
	pmode.start();
	break;
      default:
	displayWriter->clear();
	break;
      }

      mode = (mode & Control::PLAY) | (event & ~mode);
      control->indicate(mode);
    }

    if (event & Control::COPY) {
      for (int i = 0; i < GRID_BYTES; i++) {
	clipboard.grid[i] = ppc.current->grid[i];
      }
    } else if (event & Control::PASTE) {
      for (int i = 0; i < GRID_BYTES; i++)
	ppc.current->grid[i] |= clipboard.grid[i];
      ppc.draw();
    } else if (event & Control::CLEAR) {
      for (int i = 0; i < GRID_BYTES; i++)
	ppc.current->grid[i] = 0;
      ppc.draw();
    }

    // unless in note mode, grid press updates grid state
    if (!(mode & Control::NOTE)) {
      while (grid->getPress(&row, &column)) {
	pad = row * GRID_W + column;
	ppc.current->grid[pad >> 3] ^= 1 << (pad & 0x7);
	ppc.draw();
      }
    }

    // tick() according to mode
    switch (mode & ~Control::PLAY) {
    case Control::NOTE:
      nmode.tick();
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
