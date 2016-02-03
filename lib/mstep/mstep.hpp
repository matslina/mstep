#ifndef MSTEP_H_dfa982498fjasdjf982093jfas
#define MSTEP_H_dfa982498fjasdjf982093jfas

#define DEFAULT_TEMPO 120
#define DEFAULT_CHANNEL 0

#ifndef MSTEP_GRID_WIDTH
#define MSTEP_GRID_WIDTH 16
#endif

#ifndef MSTEP_GRID_HEIGHT
#define MSTEP_GRID_HEIGHT 8
#endif

#define GRID_BYTES (((MSTEP_GRID_WIDTH * MSTEP_GRID_HEIGHT) / 8) + \
		    ((MSTEP_GRID_WIDTH * MSTEP_GRID_HEIGHT) % 8 ? 1 : 0))

class Grid {
 public:
  virtual bool getPress(char *row, char *column) = 0;
  virtual void draw(char *state) = 0;
};

class MIDI {
public:
  virtual void noteOn(int channel, int note, int velocity) = 0;
};

class Control {
 public:
  enum {
    PLAY = 1,
    NOTE = 2,
    TEMPO = 4,
    PATTERN = 8,
    COPY = 16,
    PASTE = 32,
    CLEAR = 64,
    SWING = 128,
    CHANNEL = 1024,
    QUIT = 2048,
  };
  virtual void indicate(int event) = 0;
  virtual int getEvent() = 0;
  virtual int getMod() = 0;
  virtual bool getSelect() = 0;
};

class Display {
 public:
  virtual void write(int row, char *msg) = 0;
  virtual void clear() = 0;
};

typedef struct pattern_t pattern_t;

struct pattern_t {
  char grid[GRID_BYTES];
  char note[MSTEP_GRID_HEIGHT];
  char velocity[MSTEP_GRID_HEIGHT];
  char active[MSTEP_GRID_HEIGHT];
  char channel;
  char activeChannel;
  char column;
  char swing;
  int swingDelay;
};

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

  pattern_t pattern[MSTEP_GRID_HEIGHT];
  pattern_t clipboard;
  int activePattern;
  char gridOverlay[GRID_BYTES];
  char gridBuf[GRID_BYTES];

  int tempo;
  void (*sleep)(unsigned long);
  unsigned long (*time)(void);
  void displayStartupSequence();
  void draw();
  void overlayVline(char column);
  void overlayHline(char row);
  char activeRow;
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
};

#endif
