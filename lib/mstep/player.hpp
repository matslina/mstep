#pragma once

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
  ProgramController *pc;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  unsigned long int nextEventTime;
  struct pattern_state allState[GRID_H];
  int playIndex;

  void noteOff(int patternIndex) {
    struct pattern_state *state = &allState[patternIndex];
    pattern_t *pattern = &pc->program.pattern[patternIndex];

    for (int i = 0; i < GRID_H; i ++) {
      if (state->activeNote[i] >= 0) {
	midi->noteOn(state->activeChannel, state->activeNote[i], 0);
	state->activeNote[i] = -1;
      }
    }
  }

  void noteOn(int patternIndex) {
    pattern_t *pattern = &pc->program.pattern[patternIndex];
    struct pattern_state *state = &allState[patternIndex];

    for (int i = 0; i < GRID_H; i ++) {
      int pad = i * GRID_W + state->column;
      if (pattern->grid[pad / 8] & (1 << (pad % 8))) {
	midi->noteOn(pattern->channel, pattern->note[i], pattern->velocity[i]);
	state->activeNote[i] = pattern->note[i];
      }
    }
    state->activeChannel = pattern->channel;
  }

public:
  Player(MIDI *midi,
	 void (*sleep)(unsigned long),
	 unsigned long (*time)(void),
	 ProgramController *pc) {
    this->midi = midi;
    this->pc = pc;
    this->sleep = sleep;
    this->time = time;

    for (int i = 0; i < GRID_H; i++)
      for (int j = 0; j < GRID_H; j++)
	allState[i].activeNote[j] = -1;
  }

  void start() {
    playIndex = pc->currentIndex;
    allState[playIndex].swingDelay = 0;
    allState[playIndex].column = 0;
    nextEventTime = time();
  }

  void stop() {
    noteOff(playIndex);
    pc->highlightColumn = -1;
    pc->draw();
  }

  unsigned int tick() {
    int pad;
    unsigned long int now;
    unsigned long int when;
    pattern_t *pattern = &pc->program.pattern[playIndex];
    struct pattern_state *state = &allState[playIndex];

    now = time();
    when = nextEventTime + state->swingDelay;
    if (when > now)
      return when - now;

    // send MIDI messages before doing anything else.
    noteOff(playIndex);
    noteOn(playIndex);

    // if playing pattern is currently displayed: update the
    // highlighted column.
    pc->highlightColumn = -1;
    if (playIndex == pc->currentIndex)
      pc->highlightColumn = state->column;
    pc->draw();

    // step one column forward. if wrapping around, start playing the
    // currently displayed pattern and bring along data so currently
    // playing notes are turned off properly.
    if (++state->column == GRID_W) {
      if (playIndex != pc->currentIndex) {
	playIndex = pc->currentIndex;
	pattern = &pc->program.pattern[playIndex];
	for (int i = 0; i < GRID_H; i++)
	  allState[playIndex].activeNote[i] = state->activeNote[i];
	allState[playIndex].activeChannel = state->activeChannel;
	state = &allState[playIndex];
      }
      state->column = 0;
    }

    // schedule next step
    nextEventTime += (240000 / pc->program.tempo) / GRID_W;
    state->swingDelay = 0;
    if (!(state->column & 1))
      state->swingDelay = (((float)(pattern->swing - 50) / 50) *
			   (240000 / pc->program.tempo) / GRID_W);

    return MAX(0, nextEventTime + state->swingDelay - time());
  }
};
