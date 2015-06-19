
// Trellis board matrix
// Arguments 'rows' and 'cols' refers to number
// of Trellis boards, not individual pads.
// Each pad surrounded by a 'padgap' mm gap.
// Object will be 60*rows x 60*cols mm.
// This object only covers the trellis PCBs.
module trellis(rows, cols, padgap) {
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

// Control panel
// Holds 'buttons' keyboard switches, one 16x LCD
// display and one shaft encoder.
// Parameters 'height' and 'width' refer to size
// of the control panel.
module controls(buttons, height, width) {
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
module front_panel(p) {
  pad_rect(240, 120, p, p, p, p)
    trellis(2, 4, 1);

  translate([0,130, 0])
    pad_rect(240, 37, p, p, 0, p)
      controls(6, 37, 240);
}

module wall_joint(t) {
  polygon([[-1,-1], [3, 3],
           [t, 3], [t, t + 1],
           [-1, t + 1], [-1, 0],
           [0, 0]]);
}

module wall_south(length, thickness) {
  difference() {
    square([length, thickness]);
    wall_joint(thickness);
    translate([length, 0, 0])
      mirror([1,0,0])
        wall_joint(thickness);
  }
}

module hex_spacer(size) {
  for (i=[0:3])
    rotate([0,0,i*60])
      square([size / sqrt(3), size],
             center=true);
}

module wall_west(height, thickness) {
  spacer_size = 7; //6.35;
  difference() {
    square([thickness, height]);
    wall_south(thickness*2, thickness);
    translate([thickness - 4, height - 3, 0])
      square([4, 4]);
    translate([thickness, thickness * 2, 0])
      hex_spacer(spacer_size);
    translate([thickness, height - thickness * 2, 0])
      hex_spacer(spacer_size);
  }
}

module wall_east(height, thickness) {
  translate([thickness, 0, 0])
    mirror([1, 0, 0])
      wall_west(height, thickness);
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

module bottom_panel() {
  difference() {
    square([260, 177]);
    // trellis mount holes
    for (i=[0:3]) {
      for (j=[0:1]) {
        translate([10 + 60 * i,
                   10 + 60 * j, 0]) {
          translate([15, 30])
            circle(1.5);
          translate([45, 30])
            circle(1.5);
        }
      }
    }
    // lcd mount holes
    
  }
}

translate([5, 5, 0]) {
  front_panel(10);
  translate([265, 0, 0])
    wall_east(177, 8);
  translate([265, 185, 0])
    wall_west(177, 8);
  translate([0, 185, 0])
    bottom_panel();
  translate([280, 260, 0])
    rotate([0, 0, -90])
      back_panel(21, 260 - 8);
  translate([315, 260, 0])
    rotate([0, 0, -90])
      wall_south(260, 8);
}



// bottom plate
//square([240 + 20, 120 + 37 + 20]);

// bottom wall
