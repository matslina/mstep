#ifndef PLAYMODE_HPP_kigfkamxcnmxcvowe8549kxcvklsdmcl
#define PLAYMODE_HPP_kigfkamxcnmxcvowe8549kxcvklsdmcl

#include "mode.hpp"
#include "mstep.hpp"
#include "patterncontroller.hpp"

class PlayMode {
public:
  MIDI *midi;
  PatternController *pc;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  int *tempo;
  pattern_t *playing;
  unsigned long int playNext;

  PlayMode(MIDI *midi,
	   void (*sleep)(unsigned long),
	   unsigned long (*time)(void),
	   PatternController *pc,
	   int *tempo) {
    this->midi = midi;
    this->pc = pc;
    this->sleep = sleep;
    this->time = time;
    this->tempo = tempo;
  }

  void start() {
    playing = pc->current;
    playing->swingDelay = 0;
    playNext = time();
  }

  void stop() {
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

  unsigned int tick() {
    int pad;
    unsigned long int now;
    unsigned long int when;

    now = time();
    when = playNext + playing->swingDelay;
    if (when > now)
      return when - now;

    // note off for currently playing notes
    for (int i = 0; i < GRID_H; i ++) {
      if (playing->active[i] >= 0) {
	midi->noteOn(playing->activeChannel, playing->active[i], 0);
	playing->active[i] = -1;
      }
    }

    // step one column forward. currently played pattern may not be
    // the one currently displayed, so take care when drawing those
    // columns. when wrapping around we always start playing the
    // displayed pattern.
    if (++playing->column == GRID_W) {
      if (playing != pc->current) {
	playing->column = -1;
	playing = pc->current;
	playing = pc->current;
      }
      playing->column = 0;
    }
    pc->highlightColumn = playing->column;
    if (playing != pc->current)
      pc->highlightColumn = -1;
    pc->draw();

    // note on according to the grid
    for (int i = 0; i < GRID_H; i ++) {
      pad = i * GRID_W + playing->column;
      if (playing->grid[pad / 8] & (1 << (pad % 8))) {
	midi->noteOn(playing->channel, playing->note[i], playing->velocity[i]);
	playing->active[i] = playing->note[i];
      }
    }
    playing->activeChannel = playing->channel;

    // schedule next step
    playNext += (240000 / *tempo) / GRID_W;
    playing->swingDelay = 0;
    if (!(playing->column & 1))
      playing->swingDelay = (((float)(playing->swing - 50) / 50) *
			     (240000 / *tempo) / GRID_W);

    return MAX(0, playNext + playing->swingDelay - time());
  }
};

#endif
