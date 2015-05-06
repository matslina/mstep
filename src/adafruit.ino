#include <Adafruit_Trellis.h>
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
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0, &matrix1,
                                                   &matrix2, &matrix3,
                                                   &matrix4, &matrix5,
                                                   &matrix6, &matrix7);

#define LEDAT(row, col) ((col / 4) * 16) + (col % 4) + (4 * row) + ((row / 4) * 48)
#define LEDROW(i) ((i >> 6) << 2) | ((i & 0x0f) >> 2)
#define LEDCOL(i) (((i & 0x3f) >> 4 ) << 2) | (i & 0x03)


class AdaGrid : public Grid {
  bool eventPress(char *row, char *column) {
    static bool needRead = true;
    static int i;

    if (needRead) {
      trellis.readSwitches();
      needRead = false;
      i = 0;
    }

    while (i < 16 * 8) {
      if (trellis.justPressed(i)) {
	*row = LEDROW(i);
	*column = LEDCOL(i);
	i++;
	return true;
      }
      i++;
    }

    needRead = true;
    return false;
  }

  void draw(char *state) {
    for (int i = 0; i < 16 * 8; i++) {
      if (state[i / 8] & (1 << (i % 8)))
	trellis.setLED(LEDAT((i / 16), (i % 16)));
      else
	trellis.clrLED(LEDAT((i / 16), (i % 16)));
    }
    trellis.writeDisplay();
  }

  char getWidth() {
    return 16;
  }

  char getHeight() {
    return 8;
  }
};

class AdaMIDI : public MIDI {
  void noteOn(int channel, int note, int velocity) {}
};

class AdaControl : public Control {
  bool eventShutdown() {
    return false;
  }

  bool eventPlayPause() {
    static unsigned long lastLow = 0;
    return click(PIN_PLAYPAUSE, &lastLow);
  }

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
  void write() {}
};

AdaGrid grid = AdaGrid();
AdaControl control = AdaControl();
AdaDisplay display = AdaDisplay();
AdaMIDI midi = AdaMIDI();

MStep mstep = MStep(&grid, &control, &display, &midi, &delay, &millis);

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PLAYPAUSE, INPUT);
  trellis.begin(0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77);
}

void loop() {
  Serial.print("Starting MStep\n");
  mstep.run();
  Serial.print("Stopped MStep\n");
  delay(1000);
}
