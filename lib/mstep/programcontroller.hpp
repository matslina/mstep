#pragma once

#include "util.hpp"

typedef struct pattern_t {
  char grid[GRID_BYTES];
  char note[MSTEP_GRID_HEIGHT];
  char velocity[MSTEP_GRID_HEIGHT];
  char channel;
  char swing;
} pattern_t;

typedef struct program_t {
  pattern_t pattern[MSTEP_GRID_HEIGHT];
  char scene[GRID_BYTES];
  unsigned char tempo;
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
  int currentPattern;
  char highlightColumn;
  char highlightRow;
  void draw();
  char modPattern(char delta);
  char modTempo(char delta);
  char modSwing(char delta);
  char modChannel(char delta);
  void copy();
  void paste();
  void clear();
  void updateGrid();
  void toggleSceneMode();
};
