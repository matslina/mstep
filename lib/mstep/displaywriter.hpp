#pragma once

#include "mstep.hpp"

static const char *notes[] = {"C", "C#", "D", "D#", "E", "F",
			      "F#", "G", "G#", "A", "A#", "B"};

class DisplayWriter {
private:
  unsigned char row;
  unsigned char length;
  Display *display;
  char buf[20];

public:
  DisplayWriter(Display *display);
  DisplayWriter *clear();
  DisplayWriter *integer(unsigned char i);
  DisplayWriter *string(const char *s);
  DisplayWriter *note(unsigned char value);
  DisplayWriter *cr();
  DisplayWriter *namedInteger(const char *name, unsigned char i);
};

DisplayWriter::DisplayWriter(Display *display) {
  this->display = display;
  length = 0;
  row = 0;
  buf[0] = '\0';
}

DisplayWriter *DisplayWriter::clear() {
  display->clear();
  length = 0;
  row = 0;
  buf[0] = '\0';
  return this;
}

DisplayWriter *DisplayWriter::integer(unsigned char v) {
  unsigned char n = v;

  if (v > 99) {
    buf[length++] = n / 100 + '0';
    n %= 100;
  }

  if (v > 9) {
    buf[length++] = n / 10 + '0';
    n %= 10;
  }

  buf[length++] = n + '0';
  buf[length] = '\0';

  return this;
}

DisplayWriter *DisplayWriter::string(const char *s) {
  while (*s && length < 19) {
    buf[length++] = *s;
    s++;
  }
  buf[length] = '\0';

  return this;
}

DisplayWriter *DisplayWriter::cr() {
  display->write(row++, buf);
  buf[0] = '\0';
  length = 0;
  return this;
}

DisplayWriter *DisplayWriter::note(unsigned char value) {
  string(notes[value % 12]); // name
  integer(value / 12); // octave
  string(" (");
  integer(value); // MIDI number
  string(")");
  return this;
}

DisplayWriter *DisplayWriter::namedInteger(const char *name, unsigned char i) {
  string(name);
  cr();
  string("  ");
  integer(i);
  cr();
  return this;
}
