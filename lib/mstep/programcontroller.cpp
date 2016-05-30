#include "mstep.hpp"
#include "util.hpp"
#include "programcontroller.hpp"

ProgramController::ProgramController(Grid *grid) {
  this->grid = grid;

  // defaults
  for (int i = 0; i < GRID_H; i++) {
    program.pattern[i].channel = DEFAULT_CHANNEL;
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
  for (int i = 0; i < GRID_BYTES; i++)
    program.scene[i] = 0;
  program.tempo = DEFAULT_TEMPO;
  program.swing = DEFAULT_SWING;

  for (int i = 0; i < sizeof(pattern_t); i++)
    ((char *)&clipboard)[i] = 0;

  currentPattern = 0;
  current = &program.pattern[0];
  currentGrid = current->grid;
  highlightColumn = -1;
  selectedRow = -1;
  sceneMode = false;
}

char ProgramController::modPattern(char delta) {
  if (delta) {
    currentPattern = MIN(GRID_H - 1, MAX(0, currentPattern + delta));
    current = &program.pattern[currentPattern];
    currentGrid = current->grid;
    highlightColumn = -1;
    draw();
  }
  return currentPattern;
}

static void toggleRow(char *grid, unsigned int row) {
  if (row >= 0)
    for (unsigned int i = row * GRID_W; i < (row + 1) * GRID_W; i++)
      grid[i >> 3] ^= 1 << (i & 7);
}

static void toggleCol(char *grid, unsigned int col) {
  if (col >= 0)
    for (unsigned int i = col; i < GRID_W * GRID_H; i += GRID_W)
      grid[i >> 3] ^= 1 << (i & 7);
}

void ProgramController::draw() {
  toggleCol(currentGrid, highlightColumn);
  toggleRow(currentGrid, selectedRow);

  grid->draw(currentGrid);

  toggleCol(currentGrid, highlightColumn);
  toggleRow(currentGrid, selectedRow);
}

char ProgramController::modTempo(char delta) {
  program.tempo = MIN(240, MAX(0, program.tempo + delta));
  return program.tempo;
}

char ProgramController::modSwing(char delta) {
  program.swing = MIN(75, MAX(50, program.swing + delta));
  return program.swing;
}

char ProgramController::modChannel(char delta) {
  current->channel = MIN(16, MAX(1, current->channel + delta));
  return current->channel;
}

void ProgramController::copy() {
  for (int i = 0; i < GRID_BYTES; i++) {
    clipboard.grid[i] = currentGrid[i];
  }
}

void ProgramController::paste() {
  for (int i = 0; i < GRID_BYTES; i++)
    currentGrid[i] |= clipboard.grid[i];
  draw();
}

void ProgramController::clear() {
  for (int i = 0; i < GRID_BYTES; i++)
    currentGrid[i] = 0;
  draw();
}

void ProgramController::updateGrid() {
  char row, column;
  while (grid->getPress(&row, &column)) {
    char pad = row * GRID_W + column;
    currentGrid[pad >> 3] ^= 1 << (pad & 0x7);
    draw();
  }
}

void ProgramController::toggleSceneMode() {
  sceneMode = !sceneMode;
  if (sceneMode)
    currentGrid = program.scene;
  else
    currentGrid = current->grid;
  highlightColumn = -1;
  selectedRow = -1;
  draw();
}

char ProgramController::modNote(char delta) {
  current->note[selectedRow] = MIN(127, MAX(0, current->note[selectedRow] + delta));
  return current->note[selectedRow];
}

char ProgramController::modVelocity(char delta) {
  current->velocity[selectedRow] = MIN(127, MAX(0, current->velocity[selectedRow] + delta));
  return current->velocity[selectedRow];
}
