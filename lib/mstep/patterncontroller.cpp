#include "mstep.hpp"
#include "stuff.hpp"
#include "patterncontroller.hpp"

PatternController::PatternController(Grid *grid) {
  this->grid = grid;

  // defaults
  for (int i = 0; i < GRID_H; i++) {
    pattern[i].channel = DEFAULT_CHANNEL;
    pattern[i].column = -1;
    pattern[i].swing = 50;
    for (int j = 0; j < GRID_BYTES; j++)
      pattern[i].grid[j] = 0;
    for (int j = 0; j < GRID_H; j++) {
      pattern[i].note[j] = 36 + j;
      pattern[i].velocity[j] = 127;
      pattern[i].active[j] = -1;
    }

    // hard coded volca notes
    // hack hack hack hack hack
    pattern[i].note[0] = 36;
    pattern[i].note[1] = 38;
    pattern[i].note[2] = 43;
    pattern[i].note[3] = 50;
    pattern[i].note[4] = 42;
    pattern[i].note[5] = 46;
    pattern[i].note[6] = 39;
  }

  currentIndex = 0;
  current = &pattern[0];
  highlightColumn = -1;
  highlightRow = -1;
}

void PatternController::change(int steps) {
  currentIndex = MIN(GRID_H - 1, MAX(0, currentIndex + steps));
  current = &pattern[currentIndex];
  draw();
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
