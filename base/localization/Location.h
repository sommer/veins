#ifndef LOCATION_H
#define LOCATION_H

#include "Coord.h"
class Location:public Coord {
	simtime_t timestamp;
};

#endif				/* LOCATION_H */
