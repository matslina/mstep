#include "mstep.hpp"
#include "util.hpp"
#include "programcontroller.hpp"
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
  ProgramController programController = ProgramController(grid);
  StorageController storageController = StorageController(storage);
  TempoMode tempoMode = TempoMode(&displayWriter, control, &programController);
  PatternMode patternMode = PatternMode(&displayWriter, control, &programController);
  NoteMode noteMode = NoteMode(grid, &displayWriter, control, &programController);
  LoadMode loadMode = LoadMode(&displayWriter, control, &storageController, &programController);
  SaveMode saveMode = SaveMode(&displayWriter, control, &storageController, &programController);
  Player player = Player(midi, sleep, time, &programController);
  Mode *currentMode;

  mode = 0;
  currentMode = 0;
  control->indicate(mode);
  programController.draw();
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
	currentMode = &noteMode;
	break;
      case Control::TEMPO:
	currentMode = &tempoMode;
	break;
      case Control::PATTERN:
	currentMode = &patternMode;
	break;
      case Control::LOAD:
	if (!(mode & Control::PLAY))
	  currentMode = &loadMode;
	break;
      case Control::SAVE:
	if (!(mode & Control::PLAY))
	  currentMode = &saveMode;
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

    if (event & Control::COPY)
      programController.copy();
    else if (event & Control::PASTE)
      programController.paste();
    else if (event & Control::CLEAR)
      programController.clear();

    // unless in note mode, grid press updates grid state
    if (!(mode & Control::NOTE))
      programController.updateGrid();

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
