/*
 * Code to read the values
 */

#ifndef H_READ
#define H_READ

#include <vector>

#include "data.h"
#include "blobs.h"
#include "pixels.h"

// Set this to something that can't be detected on the form
static const long long DefaultID = -1;
static const int DefaultFilled = -1;

// Data to keep about a bubble
struct Bubble
{
    int radius;
    int label;
    Coord coord;

    Bubble(int r, int l, Coord c)
        : radius(r), label(l), coord(c) { }
};

// See if the boxes are vertical
bool vertical(const std::vector<Coord>& boxes,
    const int start_box, const int end_box);

// Determine ID number from boxes 2-11
long long findID(Pixels& img, const Blobs& blobs,
    const std::vector<Coord>& boxes, const Data& data, const double min_black);

// Find which of the answers is filled
std::vector<Answer> findAnswers(Pixels& img, const Blobs& blobs,
    const std::vector<Coord>& boxes, const Data& data, const double min_black);

// Percentage of pixels that are marked with a certain label in a bubble
// ranging from 0 to 1. A negative radius (the default) will use b.radius, a
// positive radius will override the bubble's radius value.
double bubbleBlackness(const Pixels& img, const Blobs& blobs, const Bubble& b,
    const int radius = -1);

// Find all bubbles within the rectangle from p1 to p2
std::vector<Bubble> findBubbles(Pixels& img, const Blobs& blobs, const int diag,
    const Coord& a, const Coord& b);

// Used to even out the slight oddities in some bubbles.
int avgRadius(const std::vector<Bubble>& bubbles);

// Find filled bubble out of a vector of possible bubbles either aligned
// vertically or horizontally (use_x true is horizontal, otherwise vertical).
// Radius is just passed in to bubbleBlackness().
int findFilled(Pixels& img, const Blobs& blobs, const std::vector<Bubble>& bubbles,
    const int start, const double jump, const int options, double black,
    const bool use_x, const int radius = -1);

// Determine answer black from the average of all bubbles in the student ID box.
// Look for the largest jump in color, which is probably the jump from not filled
// to filled-in bubbles. Pick the middle of the jump as the black value.
double findBlack(Pixels& img, const Blobs& blobs, const std::vector<Coord>& boxes,
    const Data& data);

#endif
