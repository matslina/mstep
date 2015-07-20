/*
 * Enclosure for the MStep 4711.
 *
 * A matrix of Adafruit Trellis PCBs held up by
 * vertical supports resting on a bottom plate.
 *
 * A control panel with push buttons, an LCD and a
 * rotary encoder.
 *
 * Surrounding walls including a back wall with
 * holes for MIDI ports, DC jack and power switch.
 */


MATERIAL_THICKNESS = 3;
TAB_WIDTH = 10;


module pad_rect(w, h, pn, pe, ps, pw) {
  union () {
    translate([pw, ps, 0]) child();
    translate([0, ps, 0])
      square([pw, h]);
    translate([pw + w, 0, 0])
      square([pe, h + ps]);
    translate([0, ps + h, 0])
      square([pw + pe + w, pn]);
    square([pw + w, ps]);
  }
}

module tabbed_wall(w, h, male) {
  tw = TAB_WIDTH;
  mt = MATERIAL_THICKNESS;

  foo = (h - tw) / 2;

  difference() {
    square([w, h]);
    if (male) {
      square([mt, foo]);
      translate([0, foo + tw]) square([mt, foo]);
      translate([w - mt, 0]) square([mt, foo]);
      translate([w - mt, foo + tw]) square([mt, foo]);
    } else {
      translate([0, foo]) square([mt, tw]);
      translate([w - mt, foo]) square([mt, tw]);
    }
  }
}


module grid_top(rows, cols, padgap) {
  difference() {
    square([60 * cols, 60 * rows]);
    for (i=[0:rows*4-1])
      for (j=[0:cols*4-1])
        translate([2.5 - padgap + j * 15,
                  2.5 - padgap + i * 15,
                  0])
          square(10 + 2 * padgap);
  }
}



module grid_bottom(rows, cols) {
  mt = MATERIAL_THICKNESS;
  tw = TAB_WIDTH;

  difference() {
    square([60 * cols, 60 * rows]);

    // vertical slots
    for (i=[0:rows-1])
      for (j=[0:cols-2])
        translate([60 * j + 60 - mt / 2,
                   60 * i + 30 - tw / 2])
          square([mt, tw]);

    // horizontal slots
    for (i=[0:rows-2])
      for (j=[0:cols-1])
        translate([60 * j + 30 - tw / 2,
                   60 * i + 60 - mt / 2])
          square([tw, mt]);
  }
}

module controls_top(buttons, height, width) {
  btn_size = 18.5;
  lcd_height = 25;
  lcd_width = 70;
  shaft_size = 6.4;
  knob_size = 16;

  difference() {
    square([width, height]);
    for (i=[0:buttons-1])
      translate([0.3*btn_size + i * (btn_size + 1.5),
                 height / 2 - btn_size / 2,
                 0])
        square(btn_size);
    translate([240 -1.1 * knob_size,
               height / 2,
               0])
      circle(r=shaft_size/2);
    translate([240 - 2.2 * knob_size - lcd_width,
               height / 2 - lcd_height / 2,
               0])
      square([lcd_width, lcd_height]);

  }
}

module controls_bottom(height, width) {
  square([width, height]);
}

module MIDI() {
  circle(r=7.5);
  translate([-11.15, 0, 0])
    circle(r=1.6);
  translate([11.15, 0, 0])
    circle(r=1.6);
}

module DC() {
  difference() {
    circle(6.5);
    translate([-7, -7, 0])
      square([1, 14]);
    translate([6, -7, 0])
      square([1, 14]);
  }
}

module power_switch() {
  translate([0, -6.5])
    square([19.8, 13]);
}

module back_panel(height, width) {
  difference() {
    tabbed_wall(width, height, false);
    translate([20, height / 2, 0])
      DC();
    translate([50, height / 2, 0])
      MIDI();
    translate([85, height / 2, 0])
      MIDI();
    translate([width - 30, height / 2, 0])
      power_switch();
  }
}

module	 trellis_support(n, height, lip, upper=false) {
  mt = MATERIAL_THICKNESS;
  tw = TAB_WIDTH;

  // thickness of trellis pcb and pad
  tt = 2.5;
  // pcb interconnect slot width
  piw = 20;

  union() {
    difference() {
      square([n * 60 + 2 * lip, height]);

      // pcb bed
      translate([lip, height - tt])
        square([n * 60, 2 * tt]);

      // slots for pcb interconnects
      for(i=[0:n-1])
        translate([lip + 60 / 2 + i * 60 - piw / 2,
                   height - mt - tt])
          square([piw, 2 * mt]);

      // interlock slots
      for(i=[1:n-1])
        translate([i * 60 - mt / 2,
                   upper ? height / 2 : 0])
          square([mt, height / 2]);
    }

    // tabs for bottom plate
    for(i=[0:n-1]) {
      translate([lip + 60 / 2 + i * 60 - tw / 2,
                 -mt])
        square([tw, mt]);
    }

    // tabs for walls
    translate([-mt, height / 4])
      square([mt, height / 2]);
    translate([n *60 + 2 * lip, height / 4])
      square([mt, height / 2]);
  }
}

module the_whole_shebang(w, h) {
  w = 4;
  h = 3;
  p = 10;

  // top plate
  union() {
    pad_rect(60 * w, 60 * h, p, p, p, p)
      grid_top(h, w, 1);
    translate([0, 60 * h + p, 0])
      pad_rect(60 * w, 37, p, p, 0, p)
        controls_top(6, 37, 60 * w);
  }

  // bottom plate
  translate([350, 0])
  union() {
    pad_rect(60 * w, 60 * h, p, p, p, p)
      grid_bottom(h, w);
    translate([0, 60 * h + p, 0])
      pad_rect(60 * w, 37, p, p, 0, p)
        controls_bottom(37, 60 * w);
  }

  // trellis pcb supports
  for(i=[0:w-2])
    translate([290 + i * 27, 0, 0])
        rotate([0, 0, 90])
          trellis_support(3, 23, 2, true);
  for(i=[0:h-1])
    translate([0, 250 + 27 * i, 0])
      trellis_support(4, 23, 2, false);

  // walls
  translate([350, 250, 0])
    back_panel(23, w * 60 + p * 2);
  translate([350, 275])
    tabbed_wall(w * 60 + p * 2, 23, false);
  translate([635, 0]) rotate([0, 0, 90])
    tabbed_wall(h * 60 + p * 2 + 37, 23, true);
  translate([665, 0]) rotate([0, 0, 90])
    tabbed_wall(h * 60 + p * 2 + 37, 23, true);
}

// Ponoko P3 guide
P3w = 790;
P3h = 384;
color("red") difference() {
  square([P3w, P3h]);
  translate([0.5, 0.5, 0])
    square([P3w - 1, P3h - 1]);
}

translate([5, 5, 0]) the_whole_shebang(4, 3);
