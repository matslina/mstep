
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <string.h>
#include <stdlib.h>

#include <mstep.hpp>
#include <storagecontroller.hpp>

class MockStorage : public Storage {
public:

  MockStorage(int capacity) {
    this->cap = capacity;
    this->buf = (char *)malloc(cap);
  }

  int getCapacity() {
    return cap;
  }

  int write(int address, char *src, int n) {
    REQUIRE(address >= 0);
    REQUIRE(cap >= address + n);
    memcpy(buf + address, src, n);
    return n;
  }

  int read(int address, char *dst, int n) {
    REQUIRE(address >= 0);
    REQUIRE(cap >= address + n);
    memcpy(dst, buf + address, n);
    return n;
  }

private:
  int cap;
  char *buf;
};

program_t *distinctPrograms(int n) {
  program_t *progs = (program_t *)malloc(n * sizeof(program_t));
  for (int i = 0; i < n; i++)
    for (int j = 0; j < sizeof(program_t); j++)
      ((char *)&progs[i])[j] = rand();
  // memset(&progs[i], i, sizeof(program_t));
  return progs;
}


TEST_CASE("Storage is empty at first", "[Storage]") {
  MockStorage storage = MockStorage(1231);
  StorageController controller = StorageController(&storage);

  REQUIRE(controller.numEntries == 0);
}

TEST_CASE("Magic bytes are written on instantiation", "[Storage]") {
  MockStorage storage = MockStorage(1231);
  StorageController controller = StorageController(&storage);
  char magic[4] = {'d', 'e', 'r', 'p'};

  storage.read(0, magic, 4);
  REQUIRE(!memcmp(magic, "mstp", 4));
}


TEST_CASE("Higher capacity means more slots", "[Storage]") {
  MockStorage storage1 = MockStorage(1231);
  MockStorage storage2 = MockStorage(1231 + sizeof(program_t));
  StorageController controller1 = StorageController(&storage1);
  StorageController controller2 = StorageController(&storage2);

  REQUIRE((controller1.numSlots + 1) == controller2.numSlots);
}

TEST_CASE("Programs can be saved and loaded", "[Storage]") {
  MockStorage storage = MockStorage(sizeof(program_t) * 10);
  StorageController controller = StorageController(&storage);

  program_t *program = distinctPrograms(5);
  program_t tmp;

  SECTION ("can't load stuff when nothing's been saved") {
    REQUIRE(false == controller.loadProgram(0, &tmp));
    REQUIRE(false == controller.loadProgram(1, &tmp));
    REQUIRE(false == controller.loadProgram(2, &tmp));
  }

  SECTION ("expand to new slots must be done in order") {
    REQUIRE(false == controller.saveProgram(1, &program[1]));
    controller.saveProgram(0, &program[0]);
    REQUIRE(true == controller.saveProgram(1, &program[1]));

    REQUIRE(false == controller.saveProgram(4, &program[4]));
    controller.saveProgram(2, &program[2]);
    REQUIRE(false == controller.saveProgram(4, &program[4]));
    controller.saveProgram(3, &program[3]);
    REQUIRE(true == controller.saveProgram(4, &program[4]));
  }

  SECTION ("numEntries gets increased") {
    REQUIRE(controller.numEntries == 0);
    controller.saveProgram(0, &program[2]);
    REQUIRE(controller.numEntries == 1);
    controller.saveProgram(1, &program[2]);
    REQUIRE(controller.numEntries == 2);
    controller.saveProgram(2, &program[3]);
  }

  SECTION ("programs can be saved") {
    controller.saveProgram(0, &program[4]);
    controller.saveProgram(1, &program[3]);
    controller.saveProgram(2, &program[2]);
    controller.saveProgram(3, &program[1]);
    controller.saveProgram(4, &program[0]);
    REQUIRE(controller.numEntries == 5);

    SECTION ("saved programs can be loaded") {
      REQUIRE(true == controller.loadProgram(2, &tmp));
      REQUIRE(!memcmp(&tmp, &program[2], sizeof(program_t)));
      REQUIRE(true == controller.loadProgram(1, &tmp));
      REQUIRE(!memcmp(&tmp, &program[3], sizeof(program_t)));
      REQUIRE(true == controller.loadProgram(0, &tmp));
      REQUIRE(!memcmp(&tmp, &program[4], sizeof(program_t)));
      REQUIRE(true == controller.loadProgram(0, &tmp));
      REQUIRE(!memcmp(&tmp, &program[4], sizeof(program_t)));
      REQUIRE(true == controller.loadProgram(2, &tmp));
      REQUIRE(!memcmp(&tmp, &program[2], sizeof(program_t)));
    }

    SECTION ("saved programs survive between controllers") {
      StorageController controller2 = StorageController(&storage);
      REQUIRE(true == controller2.loadProgram(2, &tmp));
      REQUIRE(!memcmp(&tmp, &program[2], sizeof(program_t)));
      REQUIRE(true == controller2.loadProgram(0, &tmp));
      REQUIRE(!memcmp(&tmp, &program[4], sizeof(program_t)));
    }

    SECTION ("saved programs can be overwritten") {
      REQUIRE(controller.numEntries == 5);
      REQUIRE(true == controller.saveProgram(0, &program[0]));
      REQUIRE(true == controller.saveProgram(1, &program[1]));
      REQUIRE(true == controller.saveProgram(2, &program[2]));
      REQUIRE(true == controller.saveProgram(3, &program[3]));
      REQUIRE(true == controller.saveProgram(4, &program[4]));
      REQUIRE(controller.numEntries == 5);

      REQUIRE(true == controller.loadProgram(0, &tmp));
      REQUIRE(!memcmp(&tmp, &program[0], sizeof(program_t)));
      REQUIRE(true == controller.loadProgram(1, &tmp));
      REQUIRE(!memcmp(&tmp, &program[1], sizeof(program_t)));
      REQUIRE(true == controller.loadProgram(2, &tmp));
      REQUIRE(!memcmp(&tmp, &program[2], sizeof(program_t)));
      REQUIRE(true == controller.loadProgram(3, &tmp));
      REQUIRE(!memcmp(&tmp, &program[3], sizeof(program_t)));
      REQUIRE(true == controller.loadProgram(4, &tmp));
      REQUIRE(!memcmp(&tmp, &program[4], sizeof(program_t)));
    }
  }
}
