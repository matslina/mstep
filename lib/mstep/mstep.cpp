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

    // if one of the Modes is active, give it a tick()
    if (currentMode) {
      bool keepCurrentModeRunning = currentMode->tick();
      if (!keepCurrentModeRunning) {
	currentMode->stop();
	currentMode = 0;
	mode &= Control::PLAY;
	control->indicate(mode);
      }
    }

    // unless in note mode, grid press updates grid state
    if (!(mode & Control::NOTE))
      programController.updateGrid();

    event = control->getEvent();

    if (!event) continue;

    // only process events when exactly 1 bit is set and the event is
    // of a known type
    if (!event ||
	event & (event - 1) ||
	!(event & (Control::QUIT |
		   Control::COPY |
		   Control::PASTE |
		   Control::CLEAR |
		   Control::PLAY |
		   Control::NOTE |
		   Control::TEMPO |
		   Control::PATTERN |
		   Control::LOAD |
		   Control::SAVE))) {
      sleep(15);
      continue;
    }

    switch (event) {
    case Control::QUIT:
      return;
    case Control::COPY:
      programController.copy();
      break;
    case Control::PASTE:
      programController.paste();
      break;
    case Control::CLEAR:
      programController.clear();
      break;
    case Control::PLAY:
      if (mode & Control::PLAY)
	player.stop();
      else
	player.start();
      mode ^= event & Control::PLAY;
      control->indicate(mode);
      break;

    default:
      // the remaining events are all associated with a Mode.  these
      // are mutually exclusive, so the current one must be stopped.
      if (currentMode) {
	currentMode->stop();
	currentMode = 0;
      }

      // the bit logic in the head of this switch ensures that we
      // don't restart a Mode that was just stopped.
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
	// TODO: make this display something useful
	displayWriter.clear();
	break;
      }

      if (currentMode)
	currentMode->start();
      mode = (mode & Control::PLAY) | (event & ~mode);
      control->indicate(mode);
      break;
    }
  }
}
