#ifndef PLAYMODE_HPP_kigfkamxcnmxcvowe8549kxcvklsdmcl
#define PLAYMODE_HPP_kigfkamxcnmxcvowe8549kxcvklsdmcl

#include "mode.hpp"
#include "mstep.hpp"
#include "patterncontroller.hpp"

class PlayMode : Mode {
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
    playNext += (240000 / *tempo) / GRID_W;
    p->swingDelay = 0;
    if (!(p->column & 1))
      p->swingDelay = ((float)(p->swing - 50) / 50) * (240000 / *tempo) / GRID_W;

    return MAX(0, playNext + p->swingDelay - time());
  }
};

#endif
