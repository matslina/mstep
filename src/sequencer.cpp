#include <Wire.h>
#include <Adafruit_Trellis.h>

#define MOMENTARY 0
#define LATCHING 1
#define MODE LATCHING

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

#define NUMTRELLIS 8
#define numKeys (NUMTRELLIS * 16)

#define LEDAT(row, col) ((col / 4) * 16) + (col % 4) + (4 * row) + ((row / 4) * 48)
#define LEDROW(i) ((i >> 6) << 2) | ((i & 0x0f) >> 2)
#define LEDCOL(i) (((i & 0x3f) >> 4 ) << 2) | (i & 0x03)

char *msg2 = \
"                "
"         XX     "
"       XX  XX   "
"     XX   XXXX  "
"   XX   XXXXXXX "
" XX      XXXXXX "
"X             X "
"XXXXXXXXXXXXXXX ";

char *msg3 = msg2;

void scroll() {
  for (int rounds = 0; rounds < 50;rounds++) {
    for (int i = 0; i < 16; i++) {
      for (int row = 0; row < 8; row++) {
	for (int col = 0; col < 16; col++) {
	  if (msg3[row * 16 + ((col + i) % 16)] == 'X')
	    trellis.setLED(LEDAT(row, col));
	  else
	    trellis.clrLED(LEDAT(row, col));
	}
      }

      trellis.writeDisplay();
      delay(100);
    }
  }
}

void printmsg(char *m) {
  Serial.println("----------------");
  for (int i = 1; i < 129; i++) {
    Serial.print(m[i-1]);
    if (i % 16 == 0)
      Serial.println();
  }
}

void readscroll() {
  char b[128];
  char *buf = b;
  int row, col;
  int btn;

  for (int i=0; i < 128; i++)
    buf[i] = ' ';

  for (int count = 0; count < 800;) {
    delay(30);
    if (digitalRead(4) == LOW)
      break;

    if (trellis.readSwitches()) {
      printmsg(buf);
      count++;
      for (uint8_t i=0; i<numKeys; i++) {
	if (trellis.justPressed(i)) {
	  trellis.setLED(LEDAT(LEDROW(i), LEDCOL(i)));
	  trellis.writeDisplay();
	  row = LEDROW(i);
	  col = LEDCOL(i);
	  Serial.print(i);
	  Serial.print('\t');
	  Serial.print(row);
	  Serial.print('x');
	  Serial.print(col);
	  Serial.print("    ");
	  Serial.println(row*16 + col);
	  buf[row * 16 + col] = 'X';
	}
      }
    }
  }

  msg3 = buf;
  scroll();
}

void bootpattern() {
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 16; column++) {
        trellis.setLED(LEDAT(row, column));
	trellis.writeDisplay();
	delay(10);
    }
  }

  for (int column = 0; column < 16; column++) {
    for (int row = 0; row < 8; row++){
      trellis.clrLED(LEDAT(row, column));
      trellis.writeDisplay();
      delay(10);
    }
  }
}


void setup() {
  Serial.begin(9600);

  pinMode(4, INPUT);
  trellis.begin(0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77);  // only one
  readscroll();
}

void loop() {
  delay(30); // 30ms delay is required, dont remove me!
  
  if (MODE == MOMENTARY) {
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
	// if it was pressed, turn it on
	if (trellis.justPressed(i)) {
	  Serial.print("v"); Serial.println(i);
	  trellis.setLED(i);
	} 
	// if it was released, turn it off
	if (trellis.justReleased(i)) {
	  Serial.print("^"); Serial.println(i);
	  trellis.clrLED(i);
	}
      }
      // tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }
  }

  if (MODE == LATCHING) {
    // If a button was just pressed or released...
    if (trellis.readSwitches()) {
      // go through every button
      for (uint8_t i=0; i<numKeys; i++) {
        // if it was pressed...
	if (trellis.justPressed(i)) {
	  Serial.print("v"); Serial.println(i);
	  // Alternate the LED
	  if (trellis.isLED(i))
	    trellis.clrLED(i);
	  else
	    trellis.setLED(i);
        } 
      }
      // tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
    }
  }
}

