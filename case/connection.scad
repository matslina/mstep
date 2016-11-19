

module midi() {
  circle(r=7.5);
  translate([-11.15, 0, 0])
    circle(r=1.6);
  translate([11.15, 0, 0])
    circle(r=1.6);
}

module power() {
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


module ConnectionPanel(width, height) {
     difference() {
	  square([width, height]);

	  translate([height / 2, height / 2])
	       power();
	  translate([50, height / 2, 0])
	       midi();
	  translate([85, height / 2, 0])
	       midi();
	  translate([width - 30, height / 2, 0])
	       power_switch();
     }
}

ConnectionPanel(300, 30);
