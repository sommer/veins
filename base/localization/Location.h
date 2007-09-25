/***************************************************************************
 * file:        Location.h
 *
 * author:      Peterpaul Klein Haneveld
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     mixim framework
 * description: Location: class with position, timestamp and confidence
 ***************************************************************************/


#ifndef LOCATION_H
#define LOCATION_H

#include <assert.h>

#include "Coord.h"
/**
 * @brief Base class with information for localization algorithms
 */
class Location:public Coord {
public:
	Location():Coord() {}
	Location(Coord pos, simtime_t ts, double c) :
		Coord(pos), 
		timestamp(ts),
		confidence(c) {}
	bool equals(Coord other) {
		return getX() == other.getX() 
			&& getY() == other.getY() 
			&& getZ() == other.getZ();
	}
	simtime_t getTimestamp() {
		return timestamp;
	}
	simtime_t setTimestamp(simtime_t t) {
		return timestamp = t;
	}
	double getConfidence() {
		return confidence;
	}
	double setConfidence(double c) {
		return confidence = c;
	}
	void setCoordinate(Coord c) {
		setX(c.getX());
		setY(c.getY());
		assert(is3D() == c.is3D());
		if (is3D())
			setZ(c.getZ());
	}
private:
	simtime_t timestamp;
	double confidence;
};

#endif				/* LOCATION_H */
