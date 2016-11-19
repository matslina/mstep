
MATERIAL_THICKNESS = 3;
BOTTOMTAB_WIDTH = 15;
WALLTAB_WIDTH = 6;

module tabsegment(length, tabwidth) {
     ntabs = floor((length / tabwidth) / 2);

     translate([(length - (ntabs * 2 - 1) * tabwidth) / 2, 0])
	  for (i = [0 : ntabs - 1])
	       translate([i * tabwidth * 2, 0])
		    square([tabwidth, MATERIAL_THICKNESS]);
}

module inverse_tabsegment(length, tabwidth) {
     difference() {
	  square([length, MATERIAL_THICKNESS]);
	  tabsegment(length, tabwidth);
     }
}


module WithBottomSlots(width, height) {
     union() {
	  translate([MATERIAL_THICKNESS, MATERIAL_THICKNESS])
	       children();

	  neww = width + MATERIAL_THICKNESS * 2;
	  newh = height + MATERIAL_THICKNESS * 2;

	  inverse_tabsegment(neww, BOTTOMTAB_WIDTH);
	  translate([0, height + MATERIAL_THICKNESS])
	       inverse_tabsegment(neww, BOTTOMTAB_WIDTH);
	  translate([MATERIAL_THICKNESS, 0])
	       rotate([0, 0, 90])
	       inverse_tabsegment(newh, BOTTOMTAB_WIDTH);
	  translate([MATERIAL_THICKNESS * 2 + width, 0])
	       rotate([0, 0, 90])
	       inverse_tabsegment(newh, BOTTOMTAB_WIDTH);
     }
}

module WithWallSlots(width, height, invert_interlock=false) {
     translate([MATERIAL_THICKNESS, MATERIAL_THICKNESS])
	  children();
     tabsegment(width + 2 * MATERIAL_THICKNESS, BOTTOMTAB_WIDTH);

     for (i = [0 : 1]) {
	  translate([i * (width + MATERIAL_THICKNESS), MATERIAL_THICKNESS]) {
	       translate([MATERIAL_THICKNESS, 0]) {
		    rotate([0, 0, 90]) {
			 if (invert_interlock)
			      tabsegment(height, WALLTAB_WIDTH);
			 else
			      inverse_tabsegment(height, WALLTAB_WIDTH);
		    }
	       }
	  }
     }
}

module WithPadding(width, height, pad) {
     translate([pad, pad]){
	  children();
     }
     square([width + pad * 2, pad]);
     square([pad, height + pad * 2]);
     translate([width + pad, 0])
	  square([pad, height + pad * 2]);
     translate([0, height + pad])
	  square([width + pad * 2, pad]);
}

translate([0, -200]) {
     WithBottomSlots(300, 300)
	  color("red")
	  square([300, 300]);

     translate([0, 310])
	  WithWallSlots(300, 30, false)
	  color("red")
	  square([300, 30]);

     translate([310, 310])
	  WithWallSlots(300, 30, true)
	  color("red")
	  square([300, 30]);
}
