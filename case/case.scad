
MATERIAL_THICKNESS = 3;
TAB_WIDTH = 20;

// Trellis board matrix
// Arguments 'rows' and 'cols' refers to number
// of Trellis boards, not individual pads.
// Each pad surrounded by a 'padgap' mm gap.
// Object will be 60*rows x 60*cols mm.
// This object only covers the trellis PCBs.
module trellis_top(rows, cols, padgap) {
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

module trellis_bottom(rows, cols) {
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


// Control panel
// Holds 'buttons' keyboard switches, one 16x LCD
// display and one shaft encoder.
// Parameters 'height' and 'width' refer to size
// of the control panel.
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


// Adds padding around a rectangular object.
// Object must be of width 'w' and height 'h'.
// Padding is added on the north, east, south and
// west sides according to 'pn', 'pe', 'ps' and
// 'pw' respectively.
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

// The complete mstep front panel
// With padding 'p'.
module mstep_top(w, h, p) {
  pad_rect(60 * w, 60 * h, p, p, p, p)
    trellis_top(3, 4, 1);

  translate([0, 60 * h + p, 0])
    pad_rect(60 * w, 37, p, p, 0, p)
      controls_top(6, 37, 60 * w);
}

// The complete mstep bottom panel
// With padding 'p'.
module mstep_bottom(w, h, p) {
  pad_rect(60 * w, 60 * h, p, p, p, p)
    trellis_bottom(h, w);

  translate([0, 60 * h + p, 0])
    pad_rect(60 * w, 37, p, p, 0, p)
      controls_bottom(37, 60 * w);
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
    square([width, height]);
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
  material_thickness = 3;
  trellis_thickness = 2.5;
  tab_width = 20;

  union() {
    difference() {
      square([n * 60 + 2 * lip,
              height]);
      translate([lip, height - trellis_thickness])
        square([n * 60, 2 * trellis_thickness]);
      for(i=[0:n-1])
        translate([lip + 60 / 2 + i * 60 - tab_width / 2,
                   height- material_thickness - trellis_thickness])
          square([tab_width, 2 * material_thickness]);
      for(i=[1:n-1])
        translate([i * 60 - material_thickness / 2,
                   upper ? height / 2 : 0])
          square([material_thickness, height / 2]);
    }
    for(i=[0:n-1]) {
      translate([lip + 60 / 2 + i * 60 - tab_width / 2,
                 - material_thickness])
        square([tab_width, material_thickness]);
    }
    translate([-material_thickness, height / 4])
      square([material_thickness, height / 2]);
    translate([n *60 + 2 * lip, height / 4])
      square([material_thickness, height / 2]);
  }
}


//back_panel(30, 169);

// power switch

// Ponoko P3 guide
P3w = 790;
P3h = 384;
color("red") difference() {
  square([P3w, P3h]);
  translate([0.5, 0.5, 0])
    square([P3w - 1, P3h - 1]);
}

translate([5, 5, 0]) {
  mstep_top(4, 3, 10);
  translate([350, 0, 0])
    mstep_bottom(4, 3, 10);
  translate([350, 250, 0])
    back_panel(23, 260 - 8);
  for(i=[0:2])
    translate([290 + i * 27, 0, 0])
        rotate([0, 0, 90])
          trellis_support(3, 23, 2, true);
  for(i=[0:1])
    translate([0, 250 + 27 * i, 0])
      trellis_support(4, 23, 2, false);
  
}



// bottom plate
//square([240 + 20, 120 + 37 + 20]);


