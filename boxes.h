/*
 * Code to determine find the boxes
 */

#ifndef H_BOXES
#define H_BOXES

#include <cmath>
#include <vector>
#include <algorithm>

#include "options.h"
#include "pixels.h"
#include "data.h"
#include "math.h"
#include "box.h"

// Functions needed due to using nested vectors
bool box_sort(const Coord& v1, const Coord& v2);

// Find boxes in the image returns { { midpoint_x, midpoint_y }, ... }
std::vector<Coord> findBoxes(Pixels& img, BoxData* data);

#endif
