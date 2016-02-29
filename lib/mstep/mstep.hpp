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
    LOAD = 128,
    SAVE = 256,
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

class Storage {
public:
  virtual int getCapacity() = 0;
  virtual int write(int address, char *src, int n) = 0;
  virtual int read(int address, char *dst, int n) = 0;
};

void mstep_run(Grid *grid, Control *control, Display *display, MIDI *midi,
	       Storage *storage,
	       void (*sleep)(unsigned long),
	       unsigned long (*time)(void));

#endif
