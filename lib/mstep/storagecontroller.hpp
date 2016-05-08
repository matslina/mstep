#pragma once

#include "mstep.hpp"
#include "patterncontroller.hpp"

#define PROGRAM_NAME_MAX 8

struct storage_header {
  char magic[4];
  int numEntries;
};

class StorageController {
private:
  Storage *storage;

public:
  int numEntries;
  int numSlots;

  StorageController(Storage *storage) {
    struct storage_header header;
    int capacity;

    storage->read(0, (char *)&header, sizeof(struct storage_header));
    if (header.magic[0] != 'm' ||
	header.magic[1] != 's' ||
	header.magic[2] != 't' ||
	header.magic[3] != 'p') {
      header.magic[0] = 'm';
      header.magic[1] = 's';
      header.magic[2] = 't';
      header.magic[3] = 'p';
      header.numEntries = 0;
      storage->write(0, (char *)&header, sizeof(struct storage_header));
    }

    capacity = storage->getCapacity();
    this->numSlots = ((capacity - sizeof(storage_header)) /
		      (sizeof(pattern_t) * GRID_H));
    this->numEntries = header.numEntries;
    this->storage = storage;
  }

  bool loadProgram(int slot, program_t *program) {
    if (slot >= numEntries)
      return false;
    storage->read(sizeof(storage_header) + slot * sizeof(program_t),
		  (char *)program, sizeof(program_t));
    return true;
  }

  bool saveProgram(int slot, program_t *program) {
    if (slot > numEntries || slot >= numSlots)
      return false;

    storage->write(sizeof(storage_header) + slot * sizeof(program_t),
		   (char *)program, sizeof(program_t));

    if (slot == numEntries) {
      numEntries++;
      struct storage_header header = {{'m', 's', 't', 'p'}, numEntries};
      storage->write(0, (char *)&header, sizeof(struct storage_header));
    }

    return true;
  }
};
