#pragma once

#include "mstep.hpp"
#include "programcontroller.hpp"
#include "displaywriter.hpp"
#include "mode.hpp"

enum ItemType {ItemTypeInteger, ItemTypeNote};

typedef struct MenuItem {
  const char *name;
  unsigned char (ProgramController::*fun)(char);
  ItemType type;
} MenuItem;

typedef struct Menu {
  MenuItem *item;
  char n;
  bool rowSelectRequired;
} Menu;


class MenuMode : public Mode {
private:
  ProgramController *programController;
  DisplayWriter *displayWriter;
  Control *control;
  Grid *grid;
  char i;
  Menu *menu;

public:
  MenuMode(Menu *menu,
	   ProgramController *programController,
	   DisplayWriter *displayWriter,
	   Control *control,
	   Grid *grid) {
    this->menu = menu;
    this->programController = programController;
    this->displayWriter = displayWriter;
    this->control = control;
    this->grid = grid;
  }

  void start() {
    i = 0;
    if (menu->rowSelectRequired)
      displayWriter->clear()->string(menu->item[i].name)->cr()->string("  <select row>")->cr();
    else
      displayWriter->clear()->namedInteger(menu->item[i].name, (programController->*(menu->item[i].fun))(0));
  }

  void stop() {
    programController->selectedRow = -1;
    programController->draw();
      }

  bool tick() {
    bool updateDisplay = false;
    char row, col;

    if (menu->rowSelectRequired) {
      row = -1;
      while (grid->getPress(&row, &col));
      if (row >= 0) {
	programController->selectedRow = row;
	programController->draw();
	updateDisplay = true;
      }

      if (programController->selectedRow < 0)
	return true;
    }

    if (control->getSelect()) {
      if (++i >= menu->n)
	i = 0;
      updateDisplay = true;
    }

    int mod = control->getMod();
    if (mod || updateDisplay) {
      int value = (programController->*(menu->item[i].fun))(mod);
      displayWriter->clear()->string(menu->item[i].name)->cr()->string("  ");
      if (menu->item[i].type == ItemTypeInteger)
	displayWriter->integer(value);
      else
	displayWriter->note(value);
      displayWriter->cr();
    }

    return true;
    }
  };

