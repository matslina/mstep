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
public:
  MIDI *midi;
  PatternController *pc;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  int *tempo;
  pattern_t *playing;
  unsigned long int playNext;
  struct pattern_state allState[GRID_H];
  struct pattern_state *state;

  Player(MIDI *midi,
	 void (*sleep)(unsigned long),
	 unsigned long (*time)(void),
	 PatternController *pc,
	 int *tempo) {
    this->midi = midi;
    this->pc = pc;
    this->sleep = sleep;
    this->time = time;
    this->tempo = tempo;

    for (int i = 0; i < GRID_H; i++) {
      for (int j = 0; j < GRID_H; j++)
	allState[i].activeNote[j] = -1;
      allState[i].column = -1;
    }
  }

  void start() {
    playing = pc->current;
    state = &allState[pc->currentIndex];
    state->swingDelay = 0;
    playNext = time();
  }

  void stop() {
    // send note off for notes currently on
    for (int i = 0; i < GRID_H; i ++) {
      if (state->activeNote[i] >= 0) {
	midi->noteOn(playing->channel, state->activeNote[i], 0);
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
    when = playNext + state->swingDelay;
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
      if (playing != pc->current) {
	state->column = -1;
	playing = pc->current;
	state = &allState[pc->currentIndex];
      }
      state->column = 0;
    }
    pc->highlightColumn = state->column;
    if (playing != pc->current)
      pc->highlightColumn = -1;
    pc->draw();

    // note on according to the grid
    for (int i = 0; i < GRID_H; i ++) {
      pad = i * GRID_W + state->column;
      if (playing->grid[pad / 8] & (1 << (pad % 8))) {
	midi->noteOn(playing->channel, playing->note[i], playing->velocity[i]);
	state->activeNote[i] = playing->note[i];
      }
    }
    state->activeChannel = playing->channel;

    // schedule next step
    playNext += (240000 / *tempo) / GRID_W;
    state->swingDelay = 0;
    if (!(state->column & 1))
      state->swingDelay = (((float)(playing->swing - 50) / 50) *
				  (240000 / *tempo) / GRID_W);

    return MAX(0, playNext + state->swingDelay - time());
  }
};

#endif
