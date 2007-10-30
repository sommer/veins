#include "Position.h"

FLOAT * coordToPosition(Coord coord) 
{
	FLOAT * pos = (FLOAT *)malloc(sizeof(Position));
	pos[0] = coord.getX();
	pos[1] = coord.getY();
	if (coord.is2D()) {
		pos[2] = 0.0;
	} else {
		pos[2] = coord.getZ();
	}
	return pos;
}

void fillPositionFromCoord(Position pos, Coord coord) 
{
	pos[0] = coord.getX();
	pos[1] = coord.getY();
	if (coord.is2D()) {
		pos[2] = 0.0;
	} else {
		pos[2] = coord.getZ();
	}
}

Coord positionToCoord(Position pos, int nr_dims)
{
	switch (nr_dims) {
	case 2: 
		return Coord(pos[0], pos[1]);
	case 3:
		return Coord(pos[0], pos[1], pos[2]);
	default:
		return Coord();
	}
}

void fillCoordFromPosition(Coord * coord, Position pos, int nr_dims)
{
	coord->setX(pos[0]);
	coord->setY(pos[1]);
	if (nr_dims != 2)
		coord->setZ(pos[2]);
}

