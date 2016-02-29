#include <LiquidCrystal.h>
#include <Adafruit_Trellis.h>

#define MSTEP_GRID_WIDTH 16
#define MSTEP_GRID_HEIGHT 16

#include <mstep.hpp>

#define PIN_PLAYPAUSE 4

Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_Trellis matrix1 = Adafruit_Trellis();
Adafruit_Trellis matrix2 = Adafruit_Trellis();
Adafruit_Trellis matrix3 = Adafruit_Trellis();
Adafruit_Trellis matrix4 = Adafruit_Trellis();
Adafruit_Trellis matrix5 = Adafruit_Trellis();
Adafruit_Trellis matrix6 = Adafruit_Trellis();
Adafruit_Trellis matrix7 = Adafruit_Trellis();
Adafruit_Trellis matrix8 = Adafruit_Trellis();
Adafruit_Trellis matrix9 = Adafruit_Trellis();
Adafruit_Trellis matrixA = Adafruit_Trellis();
Adafruit_Trellis matrixB = Adafruit_Trellis();
Adafruit_TrellisSet trellis0 = Adafruit_TrellisSet(&matrix0, &matrix1,
						   &matrix2, &matrix3);
Adafruit_TrellisSet trellis1 = Adafruit_TrellisSet(&matrix4, &matrix5,
						   &matrix6, &matrix7,
						   &matrix8, &matrix9,
						   &matrixA, &matrixB);



#define LEDAT(row, col) (((((col) >> 2) << 4) |			\
			  ((col) & 3)) |			\
			 ((row) << 2) +				\
			 ((row) >> 2) * 48)
#define LEDROW(i) (((i) >> 6) << 2) | (((i) & 0x0f) >> 2)
#define LEDCOL(i) (((i) & 0x30) >> 2) | ((i) & 0x03)


class AdaGrid : public Grid {
 public:
  bool lower;

  void initialize() {
    digitalWrite(5, HIGH);
    digitalWrite(4, HIGH);
    lower = true;
    switchI2C(false);
    trellis0.begin(0x70, 0x71, 0x72, 0x73);
    switchI2C(true);
    trellis1.begin(0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77);
  }

  void switchI2C(bool lower) {
    if (lower == this->lower)
      return;

    if (lower) {
      digitalWrite(4, HIGH);
      digitalWrite(5, LOW);
    } else {
      digitalWrite(4, LOW);
      digitalWrite(5, HIGH);
    }

    this->lower = lower;
  }

  bool getPress(char *row, char *column) {
    static bool needRead = true;
    static int i;
    bool changed;

    if (needRead) {
      switchI2C(true);
      changed = trellis1.readSwitches();
      switchI2C(false);
      changed = changed || trellis0.readSwitches();
      needRead = false;
      i = 0;
    }

    if (!changed)
      return false;

    switchI2C(false);
    while (i < 64) {
      if (trellis0.justPressed(i)) {
	*row = LEDROW(i);
	*column = LEDCOL(i);
	i++;
	return true;
      }
      i++;
    }

    switchI2C(true);
    while (i < 16 * 12) {
      if (trellis1.justPressed(i - 64)) {
	*row = 4 + LEDROW(i - 64);
	*column = LEDCOL(i - 64);
	i++;
	return true;
      }
      i++;
    }

    needRead = true;
    return false;
  }

  static void drawRange(Adafruit_TrellisSet *trellis,
			char *state, int lo, int hi) {
    int pad;
    for (int i=lo; i < hi; i++) {
      pad = i - lo;
      if (state[i >> 3] & (1 << (i & 0x7)))
	trellis->setLED(LEDAT(pad >> 4, pad & 0xf));
      else
	trellis->clrLED(LEDAT(pad >> 4, pad & 0xf));
    }
    trellis->writeDisplay();
  }

  void draw(char *state) {
    switchI2C(false);
    drawRange(&trellis0, state, 0, 64);
    switchI2C(true);
    drawRange(&trellis1, state, 64, 192);
  }

};

class AdaMIDI : public MIDI {
 public:
  void initialize() {
    Serial1.begin(31250);
  }

  void noteOn(int channel, int note, int velocity) {
    Serial1.write((unsigned char)(0x90 | channel));
    Serial1.write((unsigned char)note);
    Serial1.write((unsigned char)velocity);
  }

};

int control_mod = 0;

static void rotaryDecode() {
  control_mod += digitalRead(6) == digitalRead(7) ? 1 : -1;
}

class AdaControl : public Control {
 public:
  bool select;

  void initialize() {
      pinMode(12, INPUT);
      pinMode(11, OUTPUT);
      pinMode(10, OUTPUT);
      pinMode(9, OUTPUT);
      pinMode(8, OUTPUT);
      pinMode(7, OUTPUT);
      pinMode(6, INPUT_PULLUP);
      pinMode(7, INPUT_PULLUP);
      attachInterrupt(4, rotaryDecode, FALLING);
      select = false;
  }

  // shifts in from 4021 w (clock, latch, data) = (10, 11, 12)
  byte shiftIn() {
    int i;
    int temp = 0;
    byte myDataIn = 0;

    digitalWrite(11, HIGH);
    delayMicroseconds(20);
    digitalWrite(11, LOW);

    for (i=7; i>=0; i--)
      {
	digitalWrite(10, 0);
	delayMicroseconds(2);
	temp = digitalRead(12);
	if (temp)
	  myDataIn = myDataIn | (1 << i);
	digitalWrite(10, 1);
      }
    return myDataIn;
  }

  void indicate(int event) {
    // clock, latch, data = 8, 11, 9
    digitalWrite(11, LOW);
    shiftOut(9, 8, MSBFIRST, event);
    digitalWrite(11, HIGH);
  }

  int getEvent() {
    byte b = this->shiftIn();
    byte ret = 0;

    return b & ~(Control::QUIT);
  }

  int getMod() {
    int mod = control_mod;
    control_mod = 0;
    return mod;
  }

  bool getSelect() {
    int select = this->select;
    this->select = false;
    return select;
  }

  // keeping this old debouncing logic around for now
  bool click(int pin, unsigned long *lastLow) {
    bool low = digitalRead(pin) == LOW;
    unsigned long now;

    if (low) {
      now = millis();

      if (!*lastLow) {
	*lastLow = now;
	return true;
      }

      *lastLow = now;
      return false;
    }

    if (now - *lastLow > 50) {
	*lastLow = 0;
    }

    return false;
  }
};

class AdaDisplay : public Display {
 public:
  LiquidCrystal *lcd;

  void initialize() {
    lcd = new LiquidCrystal(A0, A1, A2, A3, A4, A5);
    lcd->begin(16, 2);
  }

  void write(int row, char *msg) {
    lcd->setCursor(0, row);
    lcd->print(msg);
  }

  void clear() {
    lcd->clear();
  }
};

class AdaStorage : public Storage {
public:
  int getCapacity() {
    return 1231;
  }

  int write(int address, char *src, int n) {
    return 321;
  }

  int read(int address, char *dst, int n) {
    return 0;
  }
};


AdaGrid grid = AdaGrid();
AdaControl control = AdaControl();
AdaDisplay display = AdaDisplay();
AdaMIDI midi = AdaMIDI();
AdaStorage storage = AdaStorage();

void setup() {
  grid.initialize();
  display.initialize();
  control.initialize();
  midi.initialize();
}

void loop() {
  mstep_run(&grid, &control, &display, &midi, &storage, &delay, &millis);
}
