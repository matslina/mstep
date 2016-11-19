use <grid.scad>;
use <control.scad>;
use <wall.scad>;
use <connection.scad>;

MATERIAL_THICKNESS = 3;
WALL_HEIGHT = 30;
OBJECT_MARGIN = 5;
CONTROLPANEL_HEIGHT = 60;

module FrontPanel() {
     WithPadding(GridFront_Size(4, 3)[0],
		 GridFront_Size(4, 3)[1] + CONTROLPANEL_HEIGHT,
		 MATERIAL_THICKNESS) {
	  translate([0, CONTROLPANEL_HEIGHT])
	       GridFront(4, 3);
	  ControlPanelFront(GridFront_Size(4, 3)[0], CONTROLPANEL_HEIGHT);
     }
}

function FrontPanel_Size() = [GridFront_Size(4, 3)[0] + 2 * MATERIAL_THICKNESS,
			      GridFront_Size(4, 3)[1] + CONTROLPANEL_HEIGHT + 2 * MATERIAL_THICKNESS];

module BackPanel() {
     w = GridFront_Size(4, 3)[0];
     h = GridFront_Size(4, 3)[1] + CONTROLPANEL_HEIGHT;

     WithBottomSlots(w, h) {
	  translate([0, CONTROLPANEL_HEIGHT])
	       GridBase(4, 3);
	  ControlPanelBase(w, CONTROLPANEL_HEIGHT);
     }
}


function BackPanel_Size() = FrontPanel_Size();


P3w = 790;
P3h = 384;
color("red") difference() {
  square([P3w, P3h]);
  translate([0.5, 0.5, 0])
    square([P3w - 1, P3h - 1]);
}


om = OBJECT_MARGIN;

translate([om, om]) {
     FrontPanel();

     translate([FrontPanel_Size()[0] + om, 0]) {
	  BackPanel();

	  translate([BackPanel_Size()[0] + om, 0]) {
	       for (i = [0 : 4]) {
		    translate([i * (WALL_HEIGHT + om), 0])
			 GridSupportVertical(3, WALL_HEIGHT);
	       }

	       translate([0, GridFront_Size(4, 3)[1] + om])
		    for (i = [0 : 3]) {
			 translate([0, i * (WALL_HEIGHT + om)])
			      GridSupportHorizontal(4, WALL_HEIGHT);
		    }
	  }

	  translate([0, FrontPanel_Size()[1] + om]) {
	       WithWallSlots(GridFront_Size(4, 3)[1] + CONTROLPANEL_HEIGHT, WALL_HEIGHT)
		    square([GridFront_Size(4, 3)[1] + CONTROLPANEL_HEIGHT, WALL_HEIGHT]);

	       translate([0, WALL_HEIGHT + om]) {
		    WithWallSlots(GridFront_Size(4, 3)[1] + CONTROLPANEL_HEIGHT, WALL_HEIGHT)
			 square([GridFront_Size(4, 3)[1] + CONTROLPANEL_HEIGHT, WALL_HEIGHT]);
	       }
	  }
     }

     translate([0, FrontPanel_Size()[1] + om]) {
	  WithWallSlots(GridFront_Size(4, 3)[0], WALL_HEIGHT, true)
	       ConnectionPanel(GridFront_Size(4, 3)[0], WALL_HEIGHT);

	  translate([0, WALL_HEIGHT + om]) {
	       WithWallSlots(GridFront_Size(4, 3)[0], WALL_HEIGHT, true)
		    square([GridFront_Size(4, 3)[0], WALL_HEIGHT]);
	  }
     }

}

