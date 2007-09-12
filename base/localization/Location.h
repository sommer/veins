#ifndef LOCATION_H
#define LOCATION_H

#include "Coord.h"
class Location:public Coord {
public:
	Location():Coord() {}
	Location(Coord pos, simtime_t ts):Coord(pos) {
		timestamp = ts;
	}
	bool equals(Coord other) {
		return getX() == other.getX() 
			&& getY() == other.getY() 
			&& getZ() == other.getZ();
	}
	simtime_t getTimestamp() {
		return timestamp;
	}
private:
	simtime_t timestamp;
};

#endif				/* LOCATION_H */
