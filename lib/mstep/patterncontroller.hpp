#ifndef PATTERNCONTROLLER_HPP_9834871897128398
#define PATTERNCONTROLLER_HPP_9834871897128398

#include "stuff.hpp"

typedef struct pattern_t {
  char grid[GRID_BYTES];
  char note[MSTEP_GRID_HEIGHT];
  char velocity[MSTEP_GRID_HEIGHT];
  char active[MSTEP_GRID_HEIGHT];
  char channel;
  char activeChannel;
  char column;
  char swing;
  int swingDelay;
} pattern_t;


class PatternController {
public:
  PatternController(Grid *grid);
  pattern_t *current;
  int currentIndex;
  char highlightColumn;
  char highlightRow;
  void change(int steps);
  void draw();

private:
  Grid *grid;
  pattern_t pattern[GRID_H];
};


#endif
