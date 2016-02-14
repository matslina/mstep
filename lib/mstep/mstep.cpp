#include <stdlib.h>

#include "mstep.hpp"
#include "stuff.hpp"
#include "patterncontroller.hpp"
#include "displaywriter.hpp"
#include "mode.hpp"
#include "patternmode.hpp"
#include "notemode.hpp"
#include "tempomode.hpp"
#include "playmode.hpp"

#ifndef F
#define F(x) (char *)x
#endif


void mstep_run(Grid *grid, Control *control, Display *display, MIDI *midi,
	       void (*sleep)(unsigned long),
	       unsigned long (*time)(void)) {
  char row, column;
  int pad;
  int event;
  int mode;
  int sleepDuration;
  int tempo = DEFAULT_TEMPO;
  pattern_t clipboard;

  DisplayWriter displayWriter = DisplayWriter(display);
  TempoMode tmode = TempoMode(&displayWriter, control, &tempo);
  PatternController ppc = PatternController(grid);
  PatternMode pmode = PatternMode(&displayWriter, control, &ppc, GRID_H);
  NoteMode nmode = NoteMode(grid, &displayWriter, control, &ppc);
  PlayMode player = PlayMode(midi, sleep, time, &ppc, &tempo);

  displayWriter.clear()->string("initializing")->cr();
  mode = 0;
  control->indicate(mode);
  ppc.draw();
  while (grid->getPress(&row, &column));
  displayWriter.clear()->string("MStep 4711")->cr()->string("  ready")->cr();

  while (1) {

    // playback is prio 1. don't bother processing events unless we
    // have "enough" time.
    if (mode & Control::PLAY) {
      sleepDuration = player.tick();
      if (sleepDuration < 30) {
	sleep(sleepDuration);
	continue;
      }
    }
    sleep(15);

    event = control->getEvent();

    // special treatment of quit and play events, coz they're special.
    if (event & Control::QUIT)
      break;
    if (event & Control::PLAY) {
      if (mode & Control::PLAY)
	player.stop();
      else
	player.start();
      mode ^= event & Control::PLAY;
      control->indicate(mode);
    }

    // only process event if it's unambiguous and it isn't garbage
    if (event && !(event & (event - 1)) &&
	event & (Control::NOTE | Control::TEMPO |
		 Control::PATTERN)) {

      // with the exception of PLAY, all modes are mutually exclusive,
      // so we stop the current mode
      switch (mode & ~Control::PLAY) {
      case Control::NOTE:
	nmode.stop();
	break;
      }

      // start the requested mode, unless it was just stopped in which
      // case we bring back the default display
      switch (event & ~mode) {
      case Control::NOTE:
	nmode.start();
	break;
      case Control::TEMPO:
	tmode.start();
	break;
      case Control::PATTERN:
	pmode.start();
	break;
      default:
	displayWriter.clear();
	break;
      }

      mode = (mode & Control::PLAY) | (event & ~mode);
      control->indicate(mode);
    }

    if (event & Control::COPY) {
      for (int i = 0; i < GRID_BYTES; i++) {
	clipboard.grid[i] = ppc.current->grid[i];
      }
    } else if (event & Control::PASTE) {
      for (int i = 0; i < GRID_BYTES; i++)
	ppc.current->grid[i] |= clipboard.grid[i];
      ppc.draw();
    } else if (event & Control::CLEAR) {
      for (int i = 0; i < GRID_BYTES; i++)
	ppc.current->grid[i] = 0;
      ppc.draw();
    }

    // unless in note mode, grid press updates grid state
    if (!(mode & Control::NOTE)) {
      while (grid->getPress(&row, &column)) {
	pad = row * GRID_W + column;
	ppc.current->grid[pad >> 3] ^= 1 << (pad & 0x7);
	ppc.draw();
      }
    }

    // tick() according to mode
    switch (mode & ~Control::PLAY) {
    case Control::NOTE:
      nmode.tick();
      break;
    case Control::TEMPO:
      tmode.tick();
      break;
    case Control::PATTERN:
      pmode.tick();
      break;
    }
  }
}
