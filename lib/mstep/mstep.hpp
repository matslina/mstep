#ifndef MSTEP_H_dfa982498fjasdjf982093jfas
#define MSTEP_H_dfa982498fjasdjf982093jfas

#define DEFAULT_TEMPO 120
#define DEFAULT_CHANNEL 0

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
    TEMPO = 64,
    PATTERN = 128,
    CHANNEL = 256,
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

typedef struct pattern_t pattern_t;

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
  int tempo;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  void displayStartupSequence();
  void draw();
  void overlayVline(char column);
  void overlayHline(char row);
  void play(char column);
  char activeRow;
  char activeColumn;
  unsigned long int playNext;
  int playPattern;
  void playStart();
  void playStop();
  int playTick();
  void tempoStart();
  void tempoTick();
  void patternStart();
  void patternTick();
  void noteStart();
  void noteStop();
  void noteTick();
  void channelStart();
  void channelTick();
};

#endif
