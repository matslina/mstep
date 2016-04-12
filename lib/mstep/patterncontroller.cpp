#include "mstep.hpp"
#include "util.hpp"
#include "patterncontroller.hpp"

PatternController::PatternController(Grid *grid) {
  this->grid = grid;

  // defaults
  for (int i = 0; i < GRID_H; i++) {
    program.pattern[i].channel = DEFAULT_CHANNEL;
    program.pattern[i].swing = 50;
    for (int j = 0; j < GRID_BYTES; j++)
      program.pattern[i].grid[j] = 0;
    for (int j = 0; j < GRID_H; j++) {
      program.pattern[i].note[j] = 36 + j;
      program.pattern[i].velocity[j] = 127;
    }

    // hard coded volca notes
    // hack hack hack hack hack
    program.pattern[i].note[0] = 36;
    program.pattern[i].note[1] = 38;
    program.pattern[i].note[2] = 43;
    program.pattern[i].note[3] = 50;
    program.pattern[i].note[4] = 42;
    program.pattern[i].note[5] = 46;
    program.pattern[i].note[6] = 39;
  }
  program.tempo = DEFAULT_TEMPO;

  currentIndex = 0;
  current = &program.pattern[0];
  highlightColumn = -1;
  highlightRow = -1;
}

char PatternController::modPattern(char delta) {
  currentIndex = MIN(GRID_H - 1, MAX(0, currentIndex + delta));
  current = &program.pattern[currentIndex];
  draw();
  return currentIndex;
}

void PatternController::draw() {
  if (highlightColumn >= 0)
    for (int i = highlightColumn; i < GRID_W * GRID_H; i += GRID_W)
      current->grid[i >> 3] ^= 1 << (i & 7);
  if (highlightRow >= 0)
    for (int i = highlightRow * GRID_W; i < (highlightRow + 1) * GRID_W; i++)
      current->grid[i >> 3] ^= 1 << (i & 7);

  grid->draw(current->grid);

  if (highlightColumn >= 0)
    for (int i = highlightColumn; i < GRID_W * GRID_H; i += GRID_W)
      current->grid[i >> 3] ^= 1 << (i & 7);
  if (highlightRow >= 0)
    for (int i = highlightRow * GRID_W; i < (highlightRow + 1) * GRID_W; i++)
      current->grid[i >> 3] ^= 1 << (i & 7);
}

char PatternController::modTempo(char delta) {
  program.tempo = MIN(240, MAX(0, program.tempo + delta));
  return program.tempo;
}

char PatternController::modSwing(char delta) {
  current->swing = MIN(75, MAX(50, current->swing + delta));
  return current->swing;
}

char PatternController::modChannel(char delta) {
  current->channel = MIN(16, MAX(1, current->channel + delta));
  return current->channel;
}
