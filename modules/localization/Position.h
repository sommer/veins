#ifndef POSITION_H
#define POSITION_H

#include <Coord.h>

#define MAX_DIM 3

typedef float FLOAT;
typedef FLOAT Position[MAX_DIM];

FLOAT * coordToPosition(Coord coord);
void fillPositionFromCoord(Position pos, Coord coord);
Coord positionToCoord(Position pos, int nr_dims);
void fillCoordFromPosition(Coord * coord, Position pos, int nr_dims);

#endif /* POSITION_H */
