
MATERIAL_THICKNESS = 3;

module pushbutton() {
     translate([10, 10]) difference() {
	  circle(r=8);
	  translate([7.5, -5])
	       square([10, 10]);
	  translate([-8.5, -5])
	       square([1, 10]);
     }
}

function pushbutton_footprint() = [20, 20];

module rotaryencoder() {
     knob_size = 16;
     shaft_size = 6.4;
     translate([8, 8])
	  circle(r=shaft_size/2);
}

function rotaryencoder_footprint() = [16, 16];

module display() {
     square([70, 25]);
}

function display_footprint() = [70, 25];

module ControlPanelFront(width, height) {
     pad = max(display_footprint()[1] / 2,
	       rotaryencoder_footprint()[0] / 2);

     difference() {
	  square([width, height]);

	  // Rotary encoder
	  translate([pad, (height - rotaryencoder_footprint()[1]) / 2])
	       rotaryencoder();

	  // LCD display
	  translate([pad + rotaryencoder_footprint()[0] + pad,
		     (height - display_footprint()[1]) / 2])
	       display();

	  // As many buttons as we can fit in the remaining space
	  offx = pad * 3 + rotaryencoder_footprint()[0] + display_footprint()[0];
	  offy = 5;
	  remx = width - offx - pad;
	  remy = height - offy * 2;
	  cols = floor(remx / pushbutton_footprint()[0]);
	  rows = floor(remy / pushbutton_footprint()[1]);
	  spacex = (remx - cols * pushbutton_footprint()[0]) / cols;
	  spacey = (remy - rows * pushbutton_footprint()[1]) / rows;
	  for (i = [0 : cols - 1])
	       for (j = [0 : rows - 1])
		    translate([offx + i * (spacex + pushbutton_footprint()[0]),
			       offy + j * (spacey + pushbutton_footprint()[1])])
			 pushbutton();
     }
}

module ControlPanelBase(width, height) {
     square([width, height]);
}

ControlPanelFront(60 * 4 + 20, 50);

translate([0, 60]) ControlPanelFront(60 * 4 + 20, 30);

translate([0, 100]) ControlPanelFront(60 * 4 + 20, 70);

translate([0, -50 - 5]) ControlPanelBase(60 * 4 + 20, 50);
