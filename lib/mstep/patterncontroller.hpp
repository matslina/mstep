#ifndef PATTERNCONTROLLER_HPP_9834871897128398
#define PATTERNCONTROLLER_HPP_9834871897128398

#include "stuff.hpp"

typedef struct pattern_t {
  char grid[GRID_BYTES];
  char note[MSTEP_GRID_HEIGHT];
  char velocity[MSTEP_GRID_HEIGHT];
  char channel;
  char swing;
} pattern_t;

typedef struct program_t {
  pattern_t pattern[MSTEP_GRID_HEIGHT];
  unsigned char tempo;
} program_t;

class PatternController {
private:
  Grid *grid;

public:
  PatternController(Grid *grid);
  program_t program;
  pattern_t *current;
  int currentIndex;
  char highlightColumn;
  char highlightRow;
  void draw();
  char modPattern(char delta);
  char modTempo(char delta);
  char modSwing(char delta);
  char modChannel(char delta);
};


#endif
