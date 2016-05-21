#include "player.hpp"

#include <stdio.h>

void Player::noteOff(int patternIndex) {
  struct pattern_state *state = &allState[patternIndex];
  pattern_t *pattern = &pc->program.pattern[patternIndex];

  for (int i = 0; i < GRID_H; i ++) {
    if (state->activeNote[i] >= 0) {
      midi->noteOn(state->activeChannel, state->activeNote[i], 0);
      state->activeNote[i] = -1;
    }
  }
}

void Player::noteOn(int patternIndex) {
  pattern_t *pattern = &pc->program.pattern[patternIndex];
  struct pattern_state *state = &allState[patternIndex];

  for (int i = 0; i < GRID_H; i ++) {
    int pad = i * GRID_W + state->column;
    if (pattern->grid[pad / 8] & (1 << (pad % 8))) {
      midi->noteOn(pattern->channel, pattern->note[i], pattern->velocity[i]);
      state->activeNote[i] = pattern->note[i];
    }
  }
  state->activeChannel = pattern->channel;
}

Player::Player(MIDI *midi,
	       void (*sleep)(unsigned long),
	       unsigned long (*time)(void),
	       ProgramController *pc) {
  this->midi = midi;
  this->pc = pc;
  this->sleep = sleep;
  this->time = time;

  for (int i = 0; i < GRID_H; i++) {
    for (int j = 0; j < GRID_H; j++)
      allState[i].activeNote[j] = -1;
    allState[i].column = 0;
  }
  for (int i = 0; i < sizeof(sceneState.activePattern); i++)
    sceneState.activePattern[i] = 0;
  sceneModeRequested = false;
}

void Player::start() {
  playIndex = pc->currentPattern;
  allState[playIndex].column = 0;
  nextEventTime = time();
  sceneMode = sceneModeRequested;
  sceneState.column = 0;
  for (int i = 0; i < sizeof(sceneState.activePattern); i++)
    sceneState.activePattern[i] = 0;
  stepCount = 0;
  swingDelay = 0;
}

void Player::stop() {
  noteOff(playIndex);
  pc->highlightColumn = -1;
  pc->draw();
}

void Player::stepPattern(int index) {
  struct pattern_state *state = &allState[index];

  noteOff(index);
  noteOn(index);

  if (++state->column == GRID_W)
    state->column = 0;
}

unsigned int Player::tick() {
  unsigned long int now;
  unsigned long int when;

  // is it time to play the next set of notes?
  now = time();
  when = nextEventTime + swingDelay;
  if (when > now)
    return when - now;

  // if so, move tick the scene or the pattern
  if (sceneMode)
    tickScene();
  else
    tickPattern();

  // schedule next step
  nextEventTime += (240000 / pc->program.tempo) / GRID_W;
  swingDelay = 0;
  if (!(stepCount & 1))
    swingDelay = (((float)(pc->program.swing - 50) / 50) *
		  (240000 / pc->program.tempo) / GRID_W);

  return MAX(0, nextEventTime + swingDelay - time());
}

void Player::tickPattern() {
  struct pattern_state *state = &allState[playIndex];
  pattern_t *pattern = &pc->program.pattern[playIndex];

  // if playing pattern is currently displayed: update the
  // highlighted column.
  if (playIndex == pc->currentPattern) {
    pc->highlightColumn = state->column;
    pc->draw();
  }

  stepPattern(playIndex);

  // if this was the GRID_W:th step, we switch to the currently
  // displayed pattern. active note data must be copied along so
  // currently playing notes are turned off properly.
  if (++stepCount == GRID_W) {
    stepCount = 0;

    // on the GRID_W:th step we switch to playing the currently
    // displayed pattern. active note data must be copied along so
    // currently playing notes are turned off properly.
    if (playIndex != pc->currentPattern) {
      playIndex = pc->currentPattern;
      pattern = &pc->program.pattern[playIndex];
      for (int i = 0; i < GRID_H; i++)
	allState[playIndex].activeNote[i] = state->activeNote[i];
      allState[playIndex].activeChannel = state->activeChannel;
      allState[playIndex].column = 0;
    }

    // NTS: how will active notes get turned off when we've switched
    //      to scenemode?

    // switch to scenemode if requested
    if (sceneModeRequested)
      sceneMode = true;
  }
}

void Player::tickScene() {
  static FILE *f = 0;
  if (!f)
    f = fopen("derp.log", "w");

  fprintf(f, "count %d\n", stepCount);fflush(f);

  if (stepCount == 0) {
    pc->highlightColumn = sceneState.column;
    pc->draw();
  }

  if (stepCount == 0) {
    for (int i = 0; i < GRID_H; i ++) {
      if (sceneState.activePattern[i / 8] & (1 << (i % 8))) {
	fprintf(f, "noteOff(%d)\n", i);fflush(f);
	noteOff(i);
      }
      sceneState.activePattern[i / 8] &= ~(1 << (i % 8));
      int pad = i * GRID_W + sceneState.column;
      if (pc->program.scene[pad / 8] & (1 << (pad % 8))) {
	sceneState.activePattern[i / 8] |= 1 << (i % 8);
	fprintf(f, "activate %d\n", i);fflush(f);
      }
    }
  }

  for (int i = 0; i < GRID_H; i ++) {
    if (sceneState.activePattern[i / 8] & (1 << (i % 8))) {
      fprintf(f, "step %d\n", i);fflush(f);
      stepPattern(i);
    }
  }

  if (++stepCount == GRID_W) {
    stepCount = 0;

    if (++sceneState.column == GRID_W)
      sceneState.column = 0;

    if (!sceneModeRequested)
      sceneMode = false;
    else {
      pc->highlightColumn = sceneState.column;
    }
  }
}

void Player::toggleSceneMode() {
  sceneModeRequested = !sceneModeRequested;
}
