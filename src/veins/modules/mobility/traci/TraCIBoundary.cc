#include "TraCIBoundary.h"
#include <algorithm>

namespace Veins {

TraCIBoundary::TraCIBoundary() {
}

void TraCIBoundary::set(TraCICoord topleft, TraCICoord bottomright, int margin) {
	this->netbounds1 = topleft;
	this->netbounds2 = bottomright;
	this->margin = margin;
}

bool TraCIBoundary::within(const Coord& c) const {
	const Coord limit = max();
	return limit.x >= c.x && limit.y >= c.y;
}

Coord TraCIBoundary::max() const {
	return Coord(traci2omnet(netbounds2).x, traci2omnet(netbounds1).y);
}

Coord TraCIBoundary::traci2omnet(TraCICoord coord) const {
	return Coord(coord.x - netbounds1.x + margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}

std::list<Coord> TraCIBoundary::traci2omnet(const std::list<TraCICoord>& list) const {
	std::list<Coord> result;
	std::transform(list.begin(), list.end(), std::back_inserter(result), std::bind1st(std::mem_fun<Coord, TraCIBoundary, TraCICoord>(&TraCIBoundary::traci2omnet), this));
	return result;
}

TraCICoord TraCIBoundary::omnet2traci(Coord coord) const {
	return TraCICoord(coord.x + netbounds1.x - margin, (netbounds2.y - netbounds1.y) - (coord.y - netbounds1.y) + margin);
}

std::list<TraCICoord> TraCIBoundary::omnet2traci(const std::list<Coord>& list) const {
	std::list<TraCICoord> result;
	std::transform(list.begin(), list.end(), std::back_inserter(result), std::bind1st(std::mem_fun<TraCICoord, TraCIBoundary, Coord>(&TraCIBoundary::omnet2traci), this));
	return result;
}

double TraCIBoundary::traci2omnetAngle(double angle) const {
	// rotate angle so 0 is east (in TraCI's angle interpretation 0 is north, 90 is east)
	angle = 90 - angle;

	// convert to rad
	angle = angle * M_PI / 180.0;

	// normalize angle to -M_PI <= angle < M_PI
	while (angle < -M_PI) angle += 2 * M_PI;
	while (angle >= M_PI) angle -= 2 * M_PI;

	return angle;
}

double TraCIBoundary::omnet2traciAngle(double angle) const {
	// convert to degrees
	angle = angle * 180 / M_PI;

	// rotate angle so 0 is south (in OMNeT++'s angle interpretation 0 is east, 90 is north)
	angle = 90 - angle;

	// normalize angle to -180 <= angle < 180
	while (angle < -180) angle += 360;
	while (angle >= 180) angle -= 360;

	return angle;
}

} /* namespace Veins */
