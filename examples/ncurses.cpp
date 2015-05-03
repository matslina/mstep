#include <stdlib.h>
#include <pthread.h>
#include <curses.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <mstep.hpp>


#define DEFAULT_WIDTH 16
#define DEFAULT_HEIGHT 8

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

class CursesGrid : public Grid {
public:
  char rows;
  char columns;
  char width;
  char height;
  WINDOW *win;
  int cursorRow;
  int cursorColumn;
  pthread_mutex_t *mutex_curses;
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
    drawGrid();
    curs_set(2);
    placeCursor(0, 0);
  }

  bool eventPress(char *row, char *column) {
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

  char getWidth() {
    return columns;
  }

  char getHeight() {
    return rows;
  }

  void moveRight() {
    placeCursor(cursorRow, cursorColumn + 1);
  }

  void moveLeft() {
    placeCursor(cursorRow, cursorColumn - 1);
  }

  void moveUp() {
    placeCursor(cursorRow - 1, cursorColumn);
  }

  void moveDown() {
    placeCursor(cursorRow + 1, cursorColumn);
  }

  void press() {
    pressedRow = cursorRow;
    pressedColumn = cursorColumn;
  }

private:
  void placeCursor(int r, int c) {
    cursorRow = MAX(0, MIN(rows - 1, r));
    cursorColumn = MAX(0, MIN(columns - 1, c));
    pthread_mutex_lock(mutex_curses);
    wmove(win, 1 + cursorRow * 2, 1 + 2 * cursorColumn);
    wrefresh(win);
    pthread_mutex_unlock(mutex_curses);
  }

  void drawGrid() {
    box(win, 0, 0);
    for (int i = 2; i < width - 2; i += 2) {
      wmove(win, 1, i);
      wvline(win, ACS_VLINE, height - 2);
      mvwaddch(win, 0, i, ACS_TTEE);
      mvwaddch(win, height - 1, i, ACS_BTEE);
    }
    for (int i = 2; i < height - 2; i += 2) {
      wmove(win, i, 1);
      whline(win, ACS_HLINE, width - 2);
      mvwaddch(win, i, 0, ACS_LTEE);
      mvwaddch(win, i, width - 1, ACS_RTEE);
      for (int j = 2; j < width - 2; j += 2)
	mvwaddch(win, i, j, ACS_PLUS);
    }
    wrefresh(win);
  }
};

class CursesDisplay : public Display {
public:
  WINDOW *win;
  int width;
  int height;
  int rows;
  int columns;
  pthread_mutex_t *mutex_curses;

  CursesDisplay(pthread_mutex_t *mutex_curses, int x, int y, int rows, int columns) {
    this->mutex_curses = mutex_curses;
    this->rows = rows;
    this->columns = columns;
    this->width = columns + 2;
    this->height = rows + 2;
    this->win = newwin(this->height, this->width, y, x);
    box(this->win, 0, 0);
    mvwaddstr(this->win, 0, 2, "LCD");
    wrefresh(this->win);
  }

  void write() {
  }
};

class CursesControl : public Control {
public:
  bool shutdownRequested;
  bool playPauseRequested;

  CursesControl() {
    shutdownRequested = false;
    playPauseRequested = false;
  }

  bool eventShutdown() {
    return returnAndClear(&shutdownRequested);
  }

  bool eventPlayPause() {
    return returnAndClear(&playPauseRequested);
  }

  void shutdown() {
    shutdownRequested = true;
  }

  void playPause() {
    playPauseRequested = true;
  }

private:
  bool returnAndClear(bool *what) {
    bool ret = *what;
    *what = false;
    return ret;
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

void *mstep_run(void *mstep) {
  ((MStep *)mstep)->run();
}

int uiloop(int grid_rows, int grid_columns) {
  CursesGrid *grid;
  CursesDisplay *display;
  CursesControl *control;
  MStep *mstep;
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
  control = new CursesControl();
  mstep = new MStep(grid, control, display, sleepms, timems);

  pthread_create(&thread_mstep, NULL, mstep_run, mstep);

  while (1) {
    pthread_mutex_lock(&mutex_curses);
    int c =  getch();
    pthread_mutex_unlock(&mutex_curses);

    if (c == 'q' || c == 'Q') {
      control->shutdown();
      break;
    }

    switch (c) {
      // no key pressed: sleep and check again
    case ERR:
      sleepms(10);
      break;

      // arrow keys move grid cursor
    case KEY_DOWN:
      grid->moveDown();
      break;
    case KEY_UP:
      grid->moveUp();
      break;
    case KEY_LEFT:
      grid->moveLeft();
      break;
    case KEY_RIGHT:
      grid->moveRight();
      break;

    case 'p':
    case 'P':
      control->playPause();
      break;

    case ' ':
      grid->press();
      break;

    default:
      break;
    }

  }

  pthread_join(thread_mstep, NULL);
  pthread_mutex_destroy(&mutex_curses);
  delete display;
  delete grid;
}

int main(int argc, char *argv[]) {
  int width = DEFAULT_WIDTH;
  int height = DEFAULT_HEIGHT;
  pthread_t sequencer;

  /* grid dimension optionally from args */
  if (argc > 1) {
    sscanf(argv[1], "%d", &width);
    if (argc > 2)
      sscanf(argv[1], "%d", &height);
    else
      height = width;
    if (argc > 3) {
      fprintf(stderr, "Usage: %s [<width> [<height>]]\n", argv[0]);
      return 1;
    }
  }

  //CursesGrid grid = Grid();


  uiloop(height, width);


  return 0;
}
