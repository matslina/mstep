#include "mstep.hpp"
#include "stuff.hpp"
#include "patterncontroller.hpp"
#include "displaywriter.hpp"
#include "mode.hpp"
#include "patternmode.hpp"
#include "notemode.hpp"
#include "tempomode.hpp"
#include "storagecontroller.hpp"
#include "loadmode.hpp"
#include "savemode.hpp"
#include "player.hpp"


void mstep_run(Grid *grid, Control *control, Display *display, MIDI *midi,
	       Storage *storage,
	       void (*sleep)(unsigned long),
	       unsigned long (*time)(void)) {
  int event;
  int mode;
  int sleepDuration;
  int tempo = DEFAULT_TEMPO;
  pattern_t clipboard;

  DisplayWriter displayWriter = DisplayWriter(display);
  PatternController ppc = PatternController(grid);
  StorageController storageController = StorageController(storage);

  TempoMode tmode = TempoMode(&displayWriter, control, &ppc);
  PatternMode pmode = PatternMode(&displayWriter, control, &ppc);
  NoteMode nmode = NoteMode(grid, &displayWriter, control, &ppc);
  LoadMode lmode = LoadMode(&displayWriter, control, &storageController, &ppc);
  SaveMode smode = SaveMode(&displayWriter, control, &storageController, &ppc);
  Player player = Player(midi, sleep, time, &ppc);
  Mode *currentMode;

  for (int i = 0; i < sizeof(pattern_t); i++)
    ((char *)&clipboard)[i] = 0;

  mode = 0;
  currentMode = 0;
  control->indicate(mode);
  ppc.draw();
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
		 Control::PATTERN | Control::LOAD | Control::SAVE)) {

      // with the exception of PLAY, all modes are mutually exclusive,
      // so we stop the current mode
      if (currentMode) {
	currentMode->stop();
	currentMode = 0;
      }

      // start the requested mode, unless it was just stopped in which
      // case we bring back the default display
      switch (event & ~mode) {
      case Control::NOTE:
	currentMode = &nmode;
	break;
      case Control::TEMPO:
	currentMode = &tmode;
	break;
      case Control::PATTERN:
	currentMode = &pmode;
	break;
      case Control::LOAD:
	if (!(mode & Control::PLAY))
	  currentMode = &lmode;
	break;
      case Control::SAVE:
	if (!(mode & Control::PLAY))
	  currentMode = &smode;
	break;
      default:
	displayWriter.clear();
	break;
      }

      if (currentMode)
	currentMode->start();

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
      char row, column;
      while (grid->getPress(&row, &column)) {
	char pad = row * GRID_W + column;
	ppc.current->grid[pad >> 3] ^= 1 << (pad & 0x7);
	ppc.draw();
      }
    }

    // tick() according to mode
    if (currentMode) {
      if (!currentMode->tick()) {
	currentMode->stop();
	mode &= Control::PLAY;
	control->indicate(mode);
      }
    }
  }
}
