#ifndef STUFF_HPP_48328fjkdsrfj2f9jewsokf
#define STUFF_HPP_48328fjkdsrfj2f9jewsokf

#include "mstep.hpp"

#define GRID_BYTES (((MSTEP_GRID_WIDTH * MSTEP_GRID_HEIGHT) / 8) + \
		    ((MSTEP_GRID_WIDTH * MSTEP_GRID_HEIGHT) % 8 ? 1 : 0))


typedef struct pattern_t pattern_t;

struct pattern_t {
  char grid[GRID_BYTES];
  char note[MSTEP_GRID_HEIGHT];
  char velocity[MSTEP_GRID_HEIGHT];
  char active[MSTEP_GRID_HEIGHT];
  char channel;
  char activeChannel;
  char column;
  char swing;
  int swingDelay;
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

  pattern_t pattern[MSTEP_GRID_HEIGHT];
  pattern_t clipboard;
  int activePattern;
  char gridOverlay[GRID_BYTES];
  char gridBuf[GRID_BYTES];

  int tempo;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  void displayStartupSequence();
  void draw();
  void overlayVline(char column);
  void overlayHline(char row);
  char activeRow;
  unsigned long int playNext;
  int playPattern;
  void playStart();
  void playStop();
  int playTick();
  void tempoStart();
  void tempoTick();
  void patternStart();
  void patternTick();
  void noteStart();
  void noteStop();
  void noteTick();
};



#endif
