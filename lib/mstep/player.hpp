#pragma once

#include "mstep.hpp"
#include "programcontroller.hpp"

struct pattern_state {
  char activeNote[GRID_H];
  char activeChannel;
  char column;
};

struct scene_state {
  char column;
  char activePattern[GRID_H / 8 + 1];
};

class Player {
private:
  MIDI *midi;
  ProgramController *pc;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  unsigned long int nextEventTime;
  struct pattern_state allState[GRID_H];
  struct scene_state sceneState;
  int playIndex;
  bool sceneMode;
  bool sceneModeRequested;
  int stepCount;
  int swingDelay;
  void stepPattern(int patternIndex);

public:
  Player(MIDI *midi,
	 void (*sleep)(unsigned long),
	 unsigned long (*time)(void),
	 ProgramController *pc);
  void start();
  void stop();
  unsigned int tick();
  void tickPattern();
  void tickScene();
  void toggleSceneMode();
};
