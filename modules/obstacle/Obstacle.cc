//
// ObstacleControl - models obstacles that block radio transmissions
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <set>
#include "obstacle/Obstacle.h"

Obstacle::Obstacle(std::string id, double attenuationPerWall, double attenuationPerMeter) :
	id(id),
	attenuationPerWall(attenuationPerWall),
	attenuationPerMeter(attenuationPerMeter) {
}

void Obstacle::setShape(Coords shape) {
	coords = shape;
	bboxP1 = Coord(1e7, 1e7);
	bboxP2 = Coord(-1e7, -1e7);
	for (Coords::const_iterator i = coords.begin(); i != coords.end(); ++i) {
		bboxP1.setX(std::min(i->getX(), bboxP1.getX()));
		bboxP1.setY(std::min(i->getY(), bboxP1.getY()));
		bboxP2.setX(std::max(i->getX(), bboxP2.getX()));
		bboxP2.setY(std::max(i->getY(), bboxP2.getY()));
	}
}

const Obstacle::Coords& Obstacle::getShape() const {
	return coords;
}

const Coord Obstacle::getBboxP1() const {
	return bboxP1;
}

const Coord Obstacle::getBboxP2() const {
	return bboxP2;
}


namespace {

	bool isPointInObstacle(Coord point, const Obstacle& o) {
		bool isInside = false;
		const Obstacle::Coords& shape = o.getShape();
		Obstacle::Coords::const_iterator i = shape.begin();
		Obstacle::Coords::const_iterator j = (shape.rbegin()+1).base();
		for (; i != shape.end(); j = i++) {
			bool inYRangeUp = (point.getY() >= i->getY()) && (point.getY() < j->getY());
			bool inYRangeDown = (point.getY() >= j->getY()) && (point.getY() < i->getY());
			bool inYRange = inYRangeUp || inYRangeDown;
			if (!inYRange) continue;
			bool intersects = point.getX() < (i->getX() + ((point.getY() - i->getY()) * (j->getX() - i->getX()) / (j->getY() - i->getY())));
			if (!intersects) continue;
			isInside = !isInside;
		}
		return isInside;
	}

	double segmentsIntersectAt(Coord p1From, Coord p1To, Coord p2From, Coord p2To) {
		Coord p1Vec = p1To - p1From;
		Coord p2Vec = p2To - p2From;
		Coord p1p2 = p1From - p2From;

		double D = (p1Vec.getX() * p2Vec.getY() - p1Vec.getY() * p2Vec.getX());

		double p1Frac = (p2Vec.getX() * p1p2.getY() - p2Vec.getY() * p1p2.getX()) / D;
		if (p1Frac < 0 || p1Frac > 1) return -1;

		double p2Frac = (p1Vec.getX() * p1p2.getY() - p1Vec.getY() * p1p2.getX()) / D;
		if (p2Frac < 0 || p2Frac > 1) return -1;

		return p1Frac;
	}
}

double Obstacle::calculateAttenuation(const Coord& senderPos, const Coord& receiverPos) const {

	// if obstacles has neither walls nor matter: bail.
	if (getShape().size() < 2) return 1;

	// get a list of points (in [0, 1]) along the line between sender and receiver where the beam intersects with this obstacle
	std::multiset<double> intersectAt;
	bool doesIntersect = false;
	const Obstacle::Coords& shape = getShape();
	Obstacle::Coords::const_iterator i = shape.begin();
	Obstacle::Coords::const_iterator j = (shape.rbegin()+1).base();
	for (; i != shape.end(); j = i++) {
		Coord c1 = *i;
		Coord c2 = *j;

		double i = segmentsIntersectAt(senderPos, receiverPos, c1, c2);
		if (i != -1) {
			doesIntersect = true;
			intersectAt.insert(i);
		}

	}

	// if beam interacts with neither walls nor matter: bail.
	bool senderInside = isPointInObstacle(senderPos, *this);
	bool receiverInside = isPointInObstacle(receiverPos, *this);
	if (!doesIntersect && !senderInside && !receiverInside) return 1;

	// remember number of walls before messing with intersection points
	double numWalls = intersectAt.size();

	// for distance calculation, make sure every other pair of points marks transition through matter and void, respectively.
	if (senderInside) intersectAt.insert(0);
	if (receiverInside) intersectAt.insert(1);
	ASSERT((intersectAt.size() % 2) == 0);

	// sum up distances in matter.
	double fractionInObstacle = 0;
	for (std::multiset<double>::const_iterator i = intersectAt.begin(); i != intersectAt.end(); ) {
		double p1 = *(i++);
		double p2 = *(i++);
		fractionInObstacle += (p2 - p1);
	}

	// calculate attenuation
	double totalDistance = senderPos.distance(receiverPos);
	double attenuation = (attenuationPerWall * numWalls) + (attenuationPerMeter * fractionInObstacle * totalDistance);
	return pow(10.0, -attenuation/10.0);
}
