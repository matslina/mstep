#ifndef STUFF_HPP_48328fjkdsrfj2f9jewsokf
#define STUFF_HPP_48328fjkdsrfj2f9jewsokf

#include "mstep.hpp"

#define GRID_W MSTEP_GRID_WIDTH
#define GRID_H MSTEP_GRID_HEIGHT
#define GRID_BYTES (((GRID_W * GRID_H) / 8) + \
		    ((GRID_W * GRID_H) % 8 ? 1 : 0))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif
