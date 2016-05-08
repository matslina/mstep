#include "mstep.hpp"
#include "util.hpp"
#include "patterncontroller.hpp"

ProgramController::ProgramController(Grid *grid) {
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

char ProgramController::modPattern(char delta) {
  currentIndex = MIN(GRID_H - 1, MAX(0, currentIndex + delta));
  current = &program.pattern[currentIndex];
  draw();
  return currentIndex;
}

void toggleRow(char *grid, int row) {
  if (row >= 0)
    for (int i = row * GRID_W; i < (row + 1) * GRID_W; i++)
      grid[i >> 3] ^= 1 << (i & 7);
}

void toggleCol(char *grid, int col) {
  if (col >= 0)
    for (int i = col; i < GRID_W * GRID_H; i += GRID_W)
      grid[i >> 3] ^= 1 << (i & 7);
}

void ProgramController::draw() {
  toggleCol(current->grid, highlightColumn);
  toggleRow(current->grid, highlightRow);

  grid->draw(current->grid);

  toggleCol(current->grid, highlightColumn);
  toggleRow(current->grid, highlightRow);
}

char ProgramController::modTempo(char delta) {
  program.tempo = MIN(240, MAX(0, program.tempo + delta));
  return program.tempo;
}

char ProgramController::modSwing(char delta) {
  current->swing = MIN(75, MAX(50, current->swing + delta));
  return current->swing;
}

char ProgramController::modChannel(char delta) {
  current->channel = MIN(16, MAX(1, current->channel + delta));
  return current->channel;
}
