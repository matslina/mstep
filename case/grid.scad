/* Trellis PCB support thingie.
 *
 * The GridSupportVertical and GridSupportHorizontal objects are meant
 * to interlock and form a grid on top of which a matrix of Trellis
 * PCBs can rest.
 *
 * The GridSupports have beds cut out to accommodate the thickness of the
 * PCB and silicone pads. The BED_DEPTH parameter dictates the depth
 * of this bed.
 *
 * The GridSupports come with tabs so that they can be fixed in an
 * underlying bottom surface. These will be MATERIAL_THICKNESS deep,
 * and TAB_WIDTH wide. The GridSupportVerticalSlot and
 * GridSupportHorizontalSlot modules can be used to cut out slots for
 * these tabs.
 *
 * The GridSupportBase object gives the smallest base surface that can fit
 * the supports and consequently also hold the PCB. The PCB will be
 * centered in this object.
 */

BED_DEPTH = 2.5;
MATERIAL_THICKNESS = 3;
SUPPORTTAB_WIDTH = 15;

module support(n, height, flip_interlock=false) {
     PAD = MATERIAL_THICKNESS * 2;
     PCB_INTERCONNECT_WIDTH = 20;

     difference () {
	  square([height, n * 60 + 2 * PAD]);

	  // cut out the pcb bed
	  translate([0, PAD])
	       square([BED_DEPTH, n * 60]);

	  // grooves to accommodate the PCB interconnects
	  for (i = [0 : n - 1])
	       translate([0, PAD + i * 60 + 60 / 2 - PCB_INTERCONNECT_WIDTH / 2])
		    square([BED_DEPTH * 2, PCB_INTERCONNECT_WIDTH]);

	  translate([flip_interlock ? height / 2 : 0, 0]) {

	       // the lower and upper interlock slots share border with the PCB
	       translate([0, PAD])
		    square([height / 2, MATERIAL_THICKNESS]);
	       translate([0, PAD + n * 60 - MATERIAL_THICKNESS])
		    square([height / 2, MATERIAL_THICKNESS]);

	       // all other overlap two PCB borders
	       for (i = [1 : n - 1])
		    translate([0, PAD + i * 60 - MATERIAL_THICKNESS / 2])
			 square([height / 2, MATERIAL_THICKNESS]);
	  }
     }

     // add tabs for mounting in bottom plate
     for (i = [0 : n - 1])
	  translate([height, PAD + i * 60 + 60 / 2 - SUPPORTTAB_WIDTH / 2])
	       square([MATERIAL_THICKNESS, SUPPORTTAB_WIDTH]);
}

/* slots matching the supports' tabs */
module supportVerticalSlots(n) {
     PAD = MATERIAL_THICKNESS * 2;
     for (i = [0 : n - 1])
	  translate([0, PAD + (i * 60) + (60 / 2) - (SUPPORTTAB_WIDTH / 2)])
	       square([MATERIAL_THICKNESS, SUPPORTTAB_WIDTH]);
}

module supportHorizontalSlots(n) {
     mirror() rotate([0, 0, 90])
	  supportVerticalSlots(n);
}

/* Base surface for a w x h Trellis PCB grid.
 *
 * A total of w + 1 vertical and h + 1 horizontal supports together
 * with this surface forms the complete PCB support structure.
 *
 * The surface comes with "bottom slots" for attaching walls.
 */
module GridBase(w, h) {
     PAD = MATERIAL_THICKNESS * 2;

     difference() {
	  square([2 * PAD + w * 60, 2 * PAD + h * 60]);

	  translate([PAD, 0])
	       supportVerticalSlots(h);
	  translate([PAD + w * 60 - MATERIAL_THICKNESS, 0])
	       supportVerticalSlots(h);
	  for (i = [1 : w - 1])
	       translate([PAD + i * 60 - MATERIAL_THICKNESS / 2, 0])
		    supportVerticalSlots(h);

	  translate([0, PAD])
	       supportHorizontalSlots(w);
	  translate([0, PAD + h * 60 - MATERIAL_THICKNESS])
	       supportHorizontalSlots(w);
	  for (i = [1 : h - 1])
	       translate([0, PAD + i * 60 - MATERIAL_THICKNESS / 2])
		    supportHorizontalSlots(w);
     }
}

function GridBase_Size(w, h) =
     [MATERIAL_THICKNESS * 4 + w * 60,
      MATERIAL_THICKNESS * 4 + h * 60];

/* Vertical support
 */
module GridSupportVertical(n, height) {
     support(n, height, false);
}

function GridSupportVertical_Size(n, height) =
     [height + MATERIAL_THICKNESS,
      MATERIAL_THICKNESS * 4 + n * 60];

/* Horizontal support
 */
module GridSupportHorizontal(n, height) {
     mirror() rotate([0, 0, 90])
	  support(n, height, true);
}

function GridSupportHorizontal_Size(n, height) =
     [4 * MATERIAL_THICKNESS + n * 60,
      height + MATERIAL_THICKNESS];

/* Profile of a single Trellis 4x4 keypad */
module trellisProfile() {
     KEY_GAP = 1; // extra space between material and keys

     for (i = [0 : 3]) {
	  for (j = [0 : 3]) {
	       translate([2.5 + i * 15 - KEY_GAP,
			  2.5 + j * 15 - KEY_GAP])
		    square(10 + 2 * KEY_GAP);
	  }
     }
}

/* Grid cover for w x h TrellisPCB's
 */
module GridFront(w, h) {
     gfw = GridBase_Size(w, h)[0];
     gfh = GridBase_Size(w, h)[1];

     offx = (gfw - w * 60) / 2;
     offy = (gfh - h * 60) / 2;

     difference() {
	  square([gfw, gfh]);
	  translate([offx, offy])
	       for (i = [0 : w - 1])
		    for (j = [0 : h - 1])
			 translate([i * 60, j * 60])
			      trellisProfile();
     }
}

function GridFront_Size(w, h) = GridBase_Size(w, h);

GridBase(4, 3);
translate([0, -GridBase_Size(4, 3)[1] - 5]) GridFront(4, 3);
translate([0, GridBase_Size(4, 3)[1] + 5]) GridSupportHorizontal(4, 30);
translate([GridBase_Size(4, 3)[0] + 5, 0]) GridSupportVertical(3, 30);
