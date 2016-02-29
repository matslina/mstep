#include <stdlib.h>
#include <pthread.h>
#include <curses.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <mstep.hpp>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define STORAGE_PATH "mstep_ncurses_storage.dat"

using namespace std;

class CursesGrid : public Grid {
public:
  WINDOW *win;
  pthread_mutex_t *mutex_curses;
  char rows;
  char columns;
  char width;
  char height;
  int cursorRow;
  int cursorColumn;
  int pressedRow;
  int pressedColumn;

  CursesGrid(pthread_mutex_t *mutex_curses, int x, int y, int rows, int columns) {
    this->mutex_curses = mutex_curses;
    this->rows = rows;
    this->columns = columns;
    this->width = columns * 2 + 1;
    this->height = rows * 2 + 1;
    this->cursorColumn = 0;
    this->cursorRow = 0;
    this->pressedRow = -1;
    this->pressedColumn = -1;
    this->win = newwin(this->height, this->width, y, x);
    curs_set(2);
    placeCursor(0, 0);
  }

  //
  // Implementations of MStep methods
  //

  bool getPress(char *row, char *column) {
    if (pressedRow >= 0) {
      *row = pressedRow;
      *column = pressedColumn;
      pressedRow = -1;
      pressedColumn = -1;
      return true;
    }
    return false;
  }

  void draw(char *state) {
    pthread_mutex_lock(mutex_curses);

    for (int i = 0; i < width * height; i++) {
      mvwaddch(win,
	       (i / columns) * 2 + 1,
	       (i % columns) * 2 + 1,
	       state[i / 8] & (1 << (i % 8)) ? ACS_DIAMOND : ' ');
    }
    pthread_mutex_unlock(mutex_curses);
    placeCursor(cursorRow, cursorColumn);
  }

  //
  // Methods specific to the ncurses grid
  //

  void move(int updown, int leftright) {
    placeCursor(MAX(0, MIN(rows, cursorRow + updown)),
		MAX(0, MIN(columns, cursorColumn + leftright)));
  }

  void press() {
    pressedRow = cursorRow;
    pressedColumn = cursorColumn;
  }

private:

  void placeCursor(int r, int c) {
    pthread_mutex_lock(mutex_curses);
    box(win, 0, 0);

    // Horizontal lines
    for (int i = 2; i < width - 2; i += 2) {
      wmove(win, 1, i);
      wvline(win, ACS_VLINE, height - 2);
      mvwaddch(win, 0, i, ACS_TTEE);
      mvwaddch(win, height - 1, i, ACS_BTEE);
    }

    // Vertical lines
    for (int i = 2; i < height - 2; i += 2) {
      wmove(win, i, 1);
      whline(win, ACS_HLINE, width - 2);
      mvwaddch(win, i, 0, ACS_LTEE);
      mvwaddch(win, i, width - 1, ACS_RTEE);
      for (int j = 2; j < width - 2; j += 2)
	mvwaddch(win, i, j, ACS_PLUS);
    }

    // Cursor placement
    cursorRow = MAX(0, MIN(rows - 1, r));
    cursorColumn = MAX(0, MIN(columns - 1, c));
    wmove(win, 1 + cursorRow * 2, 1 + 2 * cursorColumn);

    wrefresh(win);
    pthread_mutex_unlock(mutex_curses);
  }
};


struct note {
  int number;
  int velocity;
  int channel;
};

bool noteCmp(struct note a, struct note b) {
  if (a.number < b.number)
    return true;
  if (a.number ==  b.number) {
    if (a.channel < b.channel)
      return true;
    if (a.channel == b.channel)
      if (a.velocity < b.velocity)
	return true;
  }
  return false;
}

class CursesMIDI : public MIDI {
public:
  WINDOW *win;
  pthread_mutex_t *mutex_curses;
  int rows;
  vector<struct note> *active;

  CursesMIDI(pthread_mutex_t *mutex_curses, int x, int y, int rows) {
    this->mutex_curses = mutex_curses;
    this->rows = rows;
    this->win = newwin(this->rows, 40, y, x);
    this->active = new vector<struct note>();
    box(this->win, 0, 0);
    mvwaddstr(this->win, 0, 2, "MIDI");
    wrefresh(win);
  }

  void noteOn(int channel, int number, int velocity) {
    const char *notestr[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    char buf[100];
    struct note note;;

    if (velocity == 0) {
      for (int i = 0; i < active->size(); i++)
	if (active->at(i).number == number &&
	    active->at(i).channel == channel) {
	  active->erase(active->begin() + i);
	  break;
	}
    } else {
      note.number = number;
      note.velocity = velocity;
      note.channel = channel;
      active->push_back(note);
      sort(active->begin(), active->end(), noteCmp);
    }

    werase(win);
    box(this->win, 0, 0);
    mvwaddstr(this->win, 0, 2, "MIDI");

    for (int i = 0; i < MIN(active->size(), rows - 2); i++) {
      sprintf(buf, "[%d] %s%d (%d)",
	      active->at(i).channel,
	      notestr[active->at(i).number % 12],
	      active->at(i).number / 12 - 1,
	      active->at(i).velocity);
      mvwaddstr(this->win, 1 + i, 1, buf);
    }
    if (active->size() > rows - 2)
      mvwaddstr(this->win, rows - 1, 2, "<more>");
    wrefresh(win);
  }
};

class CursesDisplay : public Display {
public:
  WINDOW *win;
  pthread_mutex_t *mutex_curses;
  int width;
  int height;
  int rows;
  int columns;

  CursesDisplay(pthread_mutex_t *mutex_curses, int x, int y, int rows, int columns) {
    this->mutex_curses = mutex_curses;
    this->rows = rows;
    this->columns = columns;
    this->width = columns + 2;
    this->height = rows + 2;
    this->win = newwin(this->height, this->width, y, x);
    clear();
  }

  void write(int row, char *msg) {
    mvwaddstr(this->win, row + 1, 1, msg);
    wrefresh(this->win);
  }

  void clear() {
    pthread_mutex_lock(mutex_curses);
    wclear(win);
    box(win, 0, 0);
    mvwaddstr(win, 0, 2, "LCD");
    wrefresh(win);
    pthread_mutex_unlock(mutex_curses);
  }
};

static const char *COMMAND[] = {"Play", "Note", "Tempo", "pAttern",
				"cOpy", "paSte", "cleaR", "Load",
				"saVe", "Quit"};
static const char COMMANDCHAR[] = {'P', 'N', 'T', 'A', 'O',
				   'S', 'R', 'L', 'V'};

class CursesControl : public Control {
public:
  pthread_mutex_t *mutex_curses;
  int x;
  int y;
  int rows;
  int event;
  int mod;
  bool select;

  CursesControl(pthread_mutex_t *mutex_curses, int x, int y, int rows) {
    this->mutex_curses = mutex_curses;
    this->x = x;
    this->y = y;
    this->rows = rows;
    event = 0;
    mod = 0;
    select = false;
    indicate(0);
  }

  int getEvent() {
    int ret = event;
    event = 0;
    return ret;
  }

  int getMod() {
    int ret = mod;
    mod = 0;
    return ret;
  }

  bool getSelect() {
    bool ret = select;
    select = false;
    return ret;
  }

  void indicate(int event) {
    int len, offset;
    pthread_mutex_lock(mutex_curses);
    offset = len = 0;
    for (int i = 0; i < sizeof(COMMANDCHAR); i++) {
      if (i % rows == 0) {
	offset += len + 1;
	len = 0;
      }
      len = MAX(strlen(COMMAND[i]), len);
      if (event & (1 << i))
	attron(A_BOLD);
      mvaddstr(y + (i % rows), x + offset, COMMAND[i]);
      attroff(A_BOLD);
    }
    refresh();
    pthread_mutex_unlock(mutex_curses);
  }

  void keyPress(int c) {
    c = tolower(c);
    for (int i = 0; i < sizeof(COMMANDCHAR); i++)
      if (tolower(COMMANDCHAR[i]) == c)
	event |= 1 << i;
    switch (tolower(c)) {
    case 'u':
      mod++;
      break;
    case 'd':
      mod--;
      break;
    case 'q':
      event |= Control::QUIT;
      break;
    case '\n':
      select = true;
      break;

    }
  }

private:
  bool returnAndClear(bool *what) {
    bool ret = *what;
    *what = false;
    return ret;
  }
};

class CursesStorage : public Storage {
public:

  int getCapacity() {
    return 2000;
  }

  void init() {
    FILE *f;
    int needed;
    char zero[256];

    f = fopen(STORAGE_PATH, "a");
    memset(zero, 0, sizeof(zero));
    for (needed = getCapacity() - ftell(f);
	 needed > 0;
	 needed -= sizeof(zero))
      fwrite(zero, 1, MIN(needed, sizeof(zero)), f);
    fclose(f);
  }

  int write(int address, char *src, int n) {
    FILE *fd = fopen(STORAGE_PATH, "r+");
    fseek(fd, address, SEEK_SET);
    fwrite(src, 1, MIN(n, MAX(0, 2000 - address)), fd);
    fclose(fd);
    return MIN(n, 2000);
  }

  int read(int address, char *dst, int n) {
    FILE *fd = fopen(STORAGE_PATH, "r");
    fseek(fd, address, SEEK_SET);
    fread(dst, 1, MIN(n, MAX(0, 2000 - address)), fd);
    fclose(fd);
    return MIN(n, 2000);
  }
};


void endwindows(void) {
  endwin();
}

void sleepms(unsigned long milliseconds) {
  struct timespec req;
  req.tv_sec = milliseconds / 1000;
  req.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&req, NULL);
}

unsigned long timems() {
  struct timeval now;
  static struct timeval *start = NULL;

  if (!start) {
    start = (struct timeval *)malloc(sizeof(struct timeval));
    gettimeofday(start, NULL);
  }

  gettimeofday(&now, NULL);

  return ((now.tv_sec - start->tv_sec) * 1000 +
	  ((signed long)now.tv_usec - (signed long)start->tv_usec) / 1000);
}

struct mstep_args {
  CursesGrid *grid;
  CursesMIDI *midi;
  CursesDisplay *display;
  CursesControl *control;
  CursesStorage *storage;
};

void *mstep_run_thread(void *args) {
  struct mstep_args *margs = (struct mstep_args *)args;
  mstep_run(margs->grid, margs->control, margs->display, margs->midi,
	    margs->storage, sleepms, timems);
}

int uiloop(int grid_rows, int grid_columns) {
  CursesGrid *grid;
  CursesMIDI *midi;
  CursesDisplay *display;
  CursesControl *control;
  CursesStorage *storage;
  pthread_t thread_mstep;
  pthread_mutex_t mutex_curses;
  int pos;

  pthread_mutex_init(&mutex_curses, NULL);

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  atexit(endwindows);

  box(stdscr, 0, 0);
  refresh();
  display = new CursesDisplay(&mutex_curses, 1, 1, 4, 16);
  grid = new CursesGrid(&mutex_curses, 1, display->height + 1,
			grid_rows, grid_columns);
  control = new CursesControl(&mutex_curses, display->width + 1, 1, display->height);
  midi = new CursesMIDI(&mutex_curses, 1, display->height + grid->height + 1, 10);
  storage = new CursesStorage();
  storage->init();

  struct mstep_args args = {grid, midi, display, control, storage};
  pthread_create(&thread_mstep, NULL, mstep_run_thread, &args);

  while (1) {
    pthread_mutex_lock(&mutex_curses);
    int c =  getch();
    pthread_mutex_unlock(&mutex_curses);

    switch (c) {
      // no key pressed: sleep and check again
    case ERR:
      sleepms(10);
      break;

      // arrow keys move grid cursor
    case KEY_DOWN:
      grid->move(1, 0);
      break;
    case KEY_UP:
      grid->move(-1, 0);
      break;
    case KEY_LEFT:
      grid->move(0, -1);
      break;
    case KEY_RIGHT:
      grid->move(0, 1);
      break;
    case ' ':
      grid->press();
      break;
    default:
      control->keyPress(c);
    }

    if (c == 'q')
      break;
  }

  pthread_join(thread_mstep, NULL);
  pthread_mutex_destroy(&mutex_curses);
  delete display;
  delete grid;
}

int main(int argc, char *argv[]) {
  pthread_t sequencer;

  uiloop(MSTEP_GRID_HEIGHT, MSTEP_GRID_WIDTH);

  return 0;
}
