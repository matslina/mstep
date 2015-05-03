#ifndef MSTEP_H_dfa982498fjasdjf982093jfas
#define MSTEP_H_dfa982498fjasdjf982093jfas

class Grid {
 public:
  virtual bool eventPress(char *row, char *column) = 0;
  virtual void draw(char *state) = 0;
  virtual char getWidth() = 0;
  virtual char getHeight() = 0;
};



class Control {
 public:
  virtual bool eventShutdown() = 0;
  virtual bool eventPlayPause() = 0;
};

class Display {
 public:
  virtual void write() = 0;
};

class MStep {
public:
  MStep(Grid *grid, Control *control, Display *display,
	void (*sleep)(unsigned long),
	unsigned long (*time)(void));
  void run();

private:
  Grid *grid;
  Control *control;
  Display *display;
  char *sequence;
  char *gridState;
  char *gridOverlay;
  char *gridBuf;
  char gridStateSize;
  char gridWidth;
  char gridHeight;
  int numPads;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  void displayStartupSequence();
  void draw();
  void overlayHline(char column);
  void overlayClear();
};

#endif
