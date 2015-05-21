#ifndef MSTEP_H_dfa982498fjasdjf982093jfas
#define MSTEP_H_dfa982498fjasdjf982093jfas

class Grid {
 public:
  virtual bool getPress(char *row, char *column) = 0;
  virtual void draw(char *state) = 0;
  virtual char getWidth() = 0;
  virtual char getHeight() = 0;
};

class MIDI {
public:
  virtual void noteOn(int channel, int note, int velocity) = 0;
};

class Control {
 public:
  enum {
    PLAY = 1,
    QUIT = 2,
    NOTE = 4,
    UP = 8,
    DOWN = 16,
    SELECT = 32,
  };
  virtual void indicate(int event) = 0;
  virtual int getEvent() = 0;
  virtual int getUp() = 0;
  virtual int getDown() = 0;
};

class Display {
 public:
  virtual void write(int row, char *msg) = 0;
  virtual void clear() = 0;
};

typedef struct pattern_s {
  char *grid;
  char *note;
  char *velocity;
  char *active;
} pattern_t;

class MStep {
public:
  MStep(Grid *grid, Control *control, Display *display, MIDI *midi,
	void (*sleep)(unsigned long),
	unsigned long (*time)(void));
  void run();

private:
  Grid *grid;
  Control *control;
  Display *display;
  MIDI *midi;
  char *buf;
  pattern_t *pattern;
  int activePattern;
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
  void play(char column);
  void noteTick();
  char noteRow;
};

#endif
