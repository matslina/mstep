#ifndef PLAYMODE_HPP_kigfkamxcnmxcvowe8549kxcvklsdmcl
#define PLAYMODE_HPP_kigfkamxcnmxcvowe8549kxcvklsdmcl

#include "mode.hpp"
#include "mstep.hpp"
#include "patterncontroller.hpp"

struct pattern_state {
  char activeNote[GRID_H];
  char activeChannel;
  char column;
  int swingDelay;
};

class Player {
private:
  MIDI *midi;
  PatternController *pc;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  pattern_t *pattern;
  unsigned long int nextEventTime;
  struct pattern_state allState[GRID_H];
  struct pattern_state *state;

public:
  Player(MIDI *midi,
	 void (*sleep)(unsigned long),
	 unsigned long (*time)(void),
	 PatternController *pc) {
    this->midi = midi;
    this->pc = pc;
    this->sleep = sleep;
    this->time = time;

    for (int i = 0; i < GRID_H; i++) {
      for (int j = 0; j < GRID_H; j++)
	allState[i].activeNote[j] = -1;
      allState[i].column = -1;
    }
  }

  void start() {
    pattern = pc->current;
    state = &allState[pc->currentIndex];
    state->swingDelay = 0;
    nextEventTime = time();
  }

  void stop() {
    // send note off for notes currently on
    for (int i = 0; i < GRID_H; i ++) {
      if (state->activeNote[i] >= 0) {
	midi->noteOn(state->activeChannel, state->activeNote[i], 0);
	state->activeNote[i] = -1;
      }
    }

    // and clear the grid
    pc->highlightColumn = -1;
    pc->draw();
    state->column = -1;
  }

  unsigned int tick() {
    int pad;
    unsigned long int now;
    unsigned long int when;

    now = time();
    when = nextEventTime + state->swingDelay;
    if (when > now)
      return when - now;

    // note off for currently playing notes
    for (int i = 0; i < GRID_H; i ++) {
      if (state->activeNote[i] >= 0) {
	midi->noteOn(state->activeChannel, state->activeNote[i], 0);
	state->activeNote[i] = -1;
      }
    }

    // step one column forward. currently played pattern may not be
    // the one currently displayed, so take care when drawing those
    // columns. when wrapping around we always start playing the
    // displayed pattern.
    if (++state->column == GRID_W) {
      if (pattern != pc->current) {
	state->column = -1;
	pattern = pc->current;
	state = &allState[pc->currentIndex];
      }
      state->column = 0;
    }
    pc->highlightColumn = state->column;
    if (pattern != pc->current)
      pc->highlightColumn = -1;
    pc->draw();

    // note on according to the grid
    for (int i = 0; i < GRID_H; i ++) {
      pad = i * GRID_W + state->column;
      if (pattern->grid[pad / 8] & (1 << (pad % 8))) {
	midi->noteOn(pattern->channel, pattern->note[i], pattern->velocity[i]);
	state->activeNote[i] = pattern->note[i];
      }
    }
    state->activeChannel = pattern->channel;

    // schedule next step
    nextEventTime += (240000 / pc->program.tempo) / GRID_W;
    state->swingDelay = 0;
    if (!(state->column & 1))
      state->swingDelay = (((float)(pattern->swing - 50) / 50) *
			   (240000 / pc->program.tempo) / GRID_W);

    return MAX(0, nextEventTime + state->swingDelay - time());
  }
};

#endif
