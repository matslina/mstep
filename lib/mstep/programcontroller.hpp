#pragma once

#include <stdint.h>
#include "util.hpp"

typedef struct pattern_t {
  char grid[GRID_BYTES];
  unsigned char note[MSTEP_GRID_HEIGHT];
  unsigned char velocity[MSTEP_GRID_HEIGHT];
  unsigned char channel;
} pattern_t;

typedef struct program_t {
  pattern_t pattern[MSTEP_GRID_HEIGHT];
  char scene[GRID_BYTES];
  unsigned char tempo;
  unsigned char swing;
} program_t;

class ProgramController {
private:
  Grid *grid;
  bool sceneMode;
  pattern_t clipboard;
  pattern_t *current;
  char *currentGrid;

public:
  ProgramController(Grid *grid);
  program_t program;
  unsigned char currentPattern;
  char highlightColumn;
  char selectedRow;
  void draw();
  unsigned char modPattern(char delta);
  unsigned char modTempo(char delta);
  unsigned char modSwing(char delta);
  unsigned char modChannel(char delta);
  unsigned char modNote(char delta);
  unsigned char modVelocity(char delta);
  void copy();
  void paste();
  void clear();
  void updateGrid();
  void toggleSceneMode();
};
