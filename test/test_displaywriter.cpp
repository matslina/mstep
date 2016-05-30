
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <string.h>
#include <stdlib.h>

#include <mstep.hpp>
#include <displaywriter.hpp>

class MockDisplay : public Display {
public:

  MockDisplay(int height, int width) {
    this->h = height;
    this->w = width;
    this->buf = (char *)malloc(width * height + 1);
    clear();
  }

  void write(int row, char *msg) {
    for (int i = 0; i < w && msg[i]; i++)
      buf[row * w + i] = msg[i];
  }

  void clear() {
    memset(buf, ' ', w * h);
    buf[w * h] = '\0';
  }

  bool equals(const char *s) {
    if (!memcmp(s, buf, w * h))
      return true;
    return false;
  }

private:
  int w;
  int h;
  char *buf;
};


TEST_CASE("Calling clear() clears the display", "[DisplayWriter]") {
  MockDisplay display = MockDisplay(2, 16);
  DisplayWriter writer = DisplayWriter(&display);

  display.write(0, "0123456789abcdef");
  display.write(1, "fedcba9876543210");

  REQUIRE(display.equals("0123456789abcdef"
			 "fedcba9876543210"));
  writer.clear();
  REQUIRE(display.equals("                "
			 "                "));
}

TEST_CASE("Integers are written correctly", "[DisplayWriter]") {
  MockDisplay display = MockDisplay(2, 16);
  DisplayWriter writer = DisplayWriter(&display);

  writer.integer(0)->cr();
  REQUIRE(display.equals("0               "
			 "                "));
  writer.clear()->integer(1)->cr();
  REQUIRE(display.equals("1               "
			 "                "));
  writer.clear()->integer(9)->cr();
  REQUIRE(display.equals("9               "
			 "                "));
  writer.clear()->integer(10)->cr();
  REQUIRE(display.equals("10              "
			 "                "));
  writer.clear()->integer(99)->cr();
  REQUIRE(display.equals("99              "
			 "                "));
  writer.clear()->integer(100)->cr();
  REQUIRE(display.equals("100             "
			 "                "));
  writer.clear()->integer(255)->cr();
  REQUIRE(display.equals("255             "
			 "                "));

  writer.clear()->integer(0)->integer(255)->integer(66)->integer(123)->cr();
  REQUIRE(display.equals("025566123       "
			 "                "));
}

TEST_CASE("Strings are written correctly", "[DisplayWriter]") {
  MockDisplay display = MockDisplay(2, 16);
  DisplayWriter writer = DisplayWriter(&display);

  writer.clear()->string("foo")->string("")->string("bar")->cr();
  REQUIRE(display.equals("foobar          "
			 "                "));
  writer.string("baz      ")->string("woofoow")->cr();
  REQUIRE(display.equals("foobar          "
			 "baz      woofoow"));
}

TEST_CASE("Notes are written correctly", "[DisplayWriter]") {
  MockDisplay display = MockDisplay(2, 16);
  DisplayWriter writer = DisplayWriter(&display);

  // all notes and all octaves
  writer.note(0)->note(13)->cr()->note(26)->note(39)->cr();
  REQUIRE(display.equals("C0 (0)C#1 (13)  "
			 "D2 (26)D#3 (39) "));
  writer.clear();
  writer.note(52)->note(65)->cr()->note(78)->note(91)->cr();
  REQUIRE(display.equals("E4 (52)F5 (65)  "
			 "F#6 (78)G7 (91) "));

  // notes that feel like potential edge cases
  writer.clear()->note(11)->note(12)->cr()->note(13)->cr();
  REQUIRE(display.equals("B0 (11)C1 (12)  "
			 "C#1 (13)        "));
  writer.clear()->note(23)->note(24)->cr()->note(25)->cr();
  REQUIRE(display.equals("B1 (23)C2 (24)  "
			 "C#2 (25)        "));

  // this one overflows the first row.
  // TODO: handle overflows for real
  writer.clear()->note(126)->note(127)->cr();
  REQUIRE(display.equals("F#10 (126)G10 (1"
			 "                "));
}

TEST_CASE("Nothing is written unless cr() is invoked", "[DisplayWriter]") {
  MockDisplay display = MockDisplay(2, 16);
  DisplayWriter writer = DisplayWriter(&display);

  writer.clear()->integer(45);
  REQUIRE(display.equals("                "
			 "                "));
  writer.string("derp");
  REQUIRE(display.equals("                "
			 "                "));
  writer.note(60);
  REQUIRE(display.equals("                "
			 "                "));
  writer.cr();
  REQUIRE(display.equals("45derpC5 (60)   "
			 "                "));

  // DisplayWriter.namedInteger() is an exception, since it writes
  // multiple lines and therefore invokes cr() itself
}

TEST_CASE("Types can be mixed", "[DisplayWriter]") {
  MockDisplay display = MockDisplay(2, 16);
  DisplayWriter writer = DisplayWriter(&display);

  writer.integer(0)->note(0)->string("0zero")->integer(123)->cr();
  writer.note(47)->integer(47)->string("4477")->cr();
  REQUIRE(display.equals("0C0 (0)0zero123 "
			 "B3 (47)474477   "));

}

TEST_CASE("namedInteger does its thing correctly", "[DisplayWriter]") {
  MockDisplay display = MockDisplay(2, 16);
  DisplayWriter writer = DisplayWriter(&display);

  writer.namedInteger("FOO", 0)->cr();
  REQUIRE(display.equals("FOO             "
			 "  0             "));

  writer.clear()->namedInteger("Blergh!!", 255)->cr();
  REQUIRE(display.equals("Blergh!!        "
			 "  255           "));
}
