#include "mstep.hpp"
#include "util.hpp"
#include "programcontroller.hpp"
#include "displaywriter.hpp"
#include "mode.hpp"
#include "storagecontroller.hpp"
#include "loadmode.hpp"
#include "savemode.hpp"
#include "player.hpp"
#include "menu.hpp"

MenuItem patternItems[] = {{.name = "PATTERN",
			    .fun = &ProgramController::modPattern,
			    .type = ItemTypeInteger},
			   {.name = "SWING",
			    .fun = &ProgramController::modSwing,
			    .type = ItemTypeInteger},
			   {.name = "CHANNEL",
			    .fun = &ProgramController::modChannel,
			    .type = ItemTypeInteger}};

Menu patternMenu = {.item = patternItems, .n = 3,
		    .rowSelectRequired = false};

MenuItem noteItems[] = {{.name = "NOTE",
			 .fun = &ProgramController::modNote,
			 .type = ItemTypeNote},
			{.name = "VELOCITY",
			 .fun = &ProgramController::modVelocity,
			 .type = ItemTypeInteger}};

Menu noteMenu = {.item = noteItems, .n = 3,
		 .rowSelectRequired = true};

MenuItem tempoItems[] = {{.name = "TEMPO",
			  .fun = &ProgramController::modTempo,
			  .type = ItemTypeInteger}};

Menu tempoMenu = {.item = tempoItems, .n = 1,
		  .rowSelectRequired = false};


void mstep_run(Grid *grid, Control *control, Display *display, MIDI *midi,
	       Storage *storage,
	       void (*sleep)(unsigned long),
	       unsigned long (*time)(void)) {
  int event;
  int flags;

  DisplayWriter displayWriter = DisplayWriter(display);
  ProgramController programController = ProgramController(grid);
  StorageController storageController = StorageController(storage);

  Player player = Player(midi, sleep, time, &programController);

  MenuMode tempoMode = MenuMode(&tempoMenu, &programController, &displayWriter, control, grid);
  MenuMode patternMode = MenuMode(&patternMenu, &programController, &displayWriter, control, grid);
  MenuMode noteMode = MenuMode(&noteMenu, &programController, &displayWriter, control, grid);
  LoadMode loadMode = LoadMode(&displayWriter, control, &storageController, &programController);
  SaveMode saveMode = SaveMode(&displayWriter, control, &storageController, &programController);

  Mode *currentMode;

  flags = 0;
  currentMode = 0;
  control->indicate(flags);
  programController.draw();
  displayWriter.clear()->string("MStep 4711")->cr()->string("  ready")->cr();

  while (1) {

    // playback is prio 1. don't bother processing events unless we
    // have "enough" time.
    if (flags & Control::PLAY) {
      unsigned int duration = player.tick();
      if (duration < 30) {
	sleep(duration);
	continue;
      }
    }

    // if one of the Modes is active, give it a tick()
    if (currentMode) {
      bool keepCurrentModeRunning = currentMode->tick();
      if (!keepCurrentModeRunning) {
	currentMode->stop();
	currentMode = 0;
	flags &= Control::PLAY;
	control->indicate(flags);
      }
    }

    // unless in note mode, grid press updates grid state
    if (!(flags & Control::NOTE))
      programController.updateGrid();

    event = control->getEvent();

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
		   Control::SAVE |
		   Control::SCENE))) {
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
      if (flags & Control::PLAY)
	player.stop();
      else
	player.start();
      flags ^= event & Control::PLAY;
      control->indicate(flags);
      break;

    case Control::SCENE:
	programController.toggleSceneMode();
	player.toggleSceneMode();

    default:
      // the remaining events are all associated with a Mode.  these
      // are mutually exclusive, so the current one must be stopped.
      if (currentMode) {
	currentMode->stop();
	currentMode = 0;
      }

      // the bit logic in the head of this switch ensures that we
      // don't restart a Mode that was just stopped.
      switch (event & ~flags) {
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
	if (!(flags & Control::PLAY))
	  currentMode = &loadMode;
	break;
      case Control::SAVE:
	if (!(flags & Control::PLAY))
	  currentMode = &saveMode;
	break;
      default:
	// TODO: make this display something useful
	displayWriter.clear();
	break;
      }

      if (currentMode)
	currentMode->start();
      flags = (flags & Control::PLAY) | (event & ~flags);
      control->indicate(flags);
      break;
    }
  }
}
