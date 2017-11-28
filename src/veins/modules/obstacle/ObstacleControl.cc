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

#include <sstream>
#include <map>
#include <set>

#include <limits>
#include <math.h>

#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

using Veins::ObstacleControl;
using Veins::VehicleObstacle;

Define_Module(Veins::ObstacleControl);

ObstacleControl::~ObstacleControl() {

}

void ObstacleControl::initialize(int stage) {
	if (stage == 1)	{
		debug = par("debug");

		obstacles.clear();
		cacheEntries.clear();

		annotations = AnnotationManagerAccess().getIfExists();
		if (annotations) {
			annotationGroup = annotations->createGroup("obstacles");
			vehicleAnnotationGroup = annotations->createGroup("vehicleObstacles");
		}

		obstaclesXml = par("obstacles");

		addFromXml(obstaclesXml);
	}
}

void ObstacleControl::finish() {
	for (Obstacles::iterator i = obstacles.begin(); i != obstacles.end(); ++i) {
		for (ObstacleGridRow::iterator j = i->begin(); j != i->end(); ++j) {
			while (j->begin() != j->end()) erase(*j->begin());
		}
	}
	obstacles.clear();
}

void ObstacleControl::handleMessage(cMessage *msg) {
	if (msg->isSelfMessage()) {
		handleSelfMsg(msg);
		return;
	}
	error("ObstacleControl doesn't handle messages from other modules");
}

void ObstacleControl::handleSelfMsg(cMessage *msg) {
	error("ObstacleControl doesn't handle self-messages");
}

void ObstacleControl::addFromXml(cXMLElement* xml) {
	std::string rootTag = xml->getTagName();
	if (rootTag != "obstacles") {
		throw cRuntimeError("Obstacle definition root tag was \"%s\", but expected \"obstacles\"", rootTag.c_str());
	}

	cXMLElementList list = xml->getChildren();
	for (cXMLElementList::const_iterator i = list.begin(); i != list.end(); ++i) {
		cXMLElement* e = *i;

		std::string tag = e->getTagName();

		if (tag == "type") {
			// <type id="building" db-per-cut="9" db-per-meter="0.4" />

			ASSERT(e->getAttribute("id"));
			std::string id = e->getAttribute("id");
			ASSERT(e->getAttribute("db-per-cut"));
			std::string perCutParS = e->getAttribute("db-per-cut");
			double perCutPar = strtod(perCutParS.c_str(), 0);
			ASSERT(e->getAttribute("db-per-meter"));
			std::string perMeterParS = e->getAttribute("db-per-meter");
			double perMeterPar = strtod(perMeterParS.c_str(), 0);

			perCut[id] = perCutPar;
			perMeter[id] = perMeterPar;

		}
		else if (tag == "poly") {

			// <poly id="building#0" type="building" color="#F00" shape="16,0 8,13.8564 -8,13.8564 -16,0 -8,-13.8564 8,-13.8564" />
			ASSERT(e->getAttribute("id"));
			std::string id = e->getAttribute("id");
			ASSERT(e->getAttribute("type"));
			std::string type = e->getAttribute("type");
			ASSERT(e->getAttribute("color"));
			std::string color = e->getAttribute("color");
			ASSERT(e->getAttribute("shape"));
			std::string shape = e->getAttribute("shape");

			Obstacle obs(id, type, getAttenuationPerCut(type), getAttenuationPerMeter(type));
			std::vector<Coord> sh;
			cStringTokenizer st(shape.c_str());
			while (st.hasMoreTokens()) {
				std::string xy = st.nextToken();
				std::vector<double> xya = cStringTokenizer(xy.c_str(), ",").asDoubleVector();
				ASSERT(xya.size() == 2);
				sh.push_back(Coord(xya[0], xya[1]));
			}
			obs.setShape(sh);
			add(obs);
		}
		else {
			throw cRuntimeError("Found unknown tag in obstacle definition: \"%s\"", tag.c_str());
		}


	}

}

void ObstacleControl::addFromTypeAndShape(std::string id, std::string typeId, std::vector<Coord> shape) {
	if (!isTypeSupported(typeId)) {
		throw cRuntimeError("Unsupported obstacle type: \"%s\"", typeId.c_str());
	}
	Obstacle obs(id, typeId, getAttenuationPerCut(typeId), getAttenuationPerMeter(typeId));
	obs.setShape(shape);
	add(obs);
}

void ObstacleControl::add(Obstacle obstacle) {
	Obstacle* o = new Obstacle(obstacle);

	size_t fromRow = std::max(0, int(o->getBboxP1().x / GRIDCELL_SIZE));
	size_t toRow = std::max(0, int(o->getBboxP2().x / GRIDCELL_SIZE));
	size_t fromCol = std::max(0, int(o->getBboxP1().y / GRIDCELL_SIZE));
	size_t toCol = std::max(0, int(o->getBboxP2().y / GRIDCELL_SIZE));
	for (size_t row = fromRow; row <= toRow; ++row) {
		for (size_t col = fromCol; col <= toCol; ++col) {
			if (obstacles.size() < col+1) obstacles.resize(col+1);
			if (obstacles[col].size() < row+1) obstacles[col].resize(row+1);
			(obstacles[col])[row].push_back(o);
		}
	}

	// visualize using AnnotationManager
	if (annotations) o->visualRepresentation = annotations->drawPolygon(o->getShape(), "red", annotationGroup);

	cacheEntries.clear();
}

void ObstacleControl::erase(const Obstacle* obstacle) {
	for (Obstacles::iterator i = obstacles.begin(); i != obstacles.end(); ++i) {
		for (ObstacleGridRow::iterator j = i->begin(); j != i->end(); ++j) {
			for (ObstacleGridCell::iterator k = j->begin(); k != j->end(); ) {
				Obstacle* o = *k;
				if (o == obstacle) {
					k = j->erase(k);
				} else {
					++k;
				}
			}
		}
	}

	if (annotations && obstacle->visualRepresentation) annotations->erase(obstacle->visualRepresentation);
	delete obstacle;

	cacheEntries.clear();
}

const VehicleObstacle* ObstacleControl::add(VehicleObstacle obstacle) {
	VehicleObstacle* o = new VehicleObstacle(obstacle);
	vehicleObstacles.push_back(o);

	return o;
}

void ObstacleControl::erase(const VehicleObstacle* obstacle) {
	bool erasedOne = false;
	for (VehicleObstacles::iterator k = vehicleObstacles.begin(); k != vehicleObstacles.end(); ) {
		VehicleObstacle* o = *k;
		if (o == obstacle) {
			erasedOne = true;
			k = vehicleObstacles.erase(k);
		} else {
			++k;
		}
	}
	ASSERT(erasedOne);
	delete obstacle;
}

namespace {
	// Calculate impact of vehicles as obstacles according to:
	// M. Boban, T. T. V. Vinhoza, M. Ferreira, J. Barros, and O. K. Tonguz: 'Impact of Vehicles as Obstacles in Vehicular Ad Hoc Networks', IEEE JSAC, Vol. 29, No. 1, January 2011


	/**
	 * compute attenuation due to (single) vehicle.
	 * @param h1: height of sender
	 * @param h2: height of receiver
	 * @param h: height of obstacle
	 * @param d: distance between sender and receiver
	 * @param d1: distance between sender and obstacle
	 * @param f: frequency of the transmission
	 */
	double getVehicleAttenuation(double h1, double h2, double h, double d, double d1, double f) {
		double lambda = 0.3 / f;
		double d2 = d - d1;
		double y = (h2 - h1) / d * d1 + h1;
		double H = h-y;
		double r1 = sqrt(lambda * d1 * d2 / d);
		double V0 = sqrt(2) * H / r1;

		if (V0 <= -0.7) return 0;

		return 6.9 + 20 * log10(sqrt(pow((V0 - 0.1), 2) + 1) + V0 - 0.1);
	}

	/**
	 * compute attenuation due to vehicles.
	 * @param XY: a vector of (distance, height) referring to potential obstacles along the line of sight, starting with the sender and ending with the receiver
	 * @param f: the frequency of the transmission
	 */
	double getVehicleAttenuation(const std::vector<std::pair<double, double> >& XY, double f) {

		// basic sanity check
		assert(XY.size() >= 2);

		// make sure the list of x coordinates is sorted
		for (size_t i = 0; i < XY.size()-1; i++) {
			assert(XY[i].first < XY[i+1].first);
		}

		// find "major obstacles" (MOs) between sender and receiver via rope-stretching algorithm
		/*
		 *      |
		 *      |         |
		 *      |   :     |
		 *  |   |   :  :  |    |
		 * mo0 mo1       mo2  mo3
		 * snd                rcv
		 */
		std::vector<size_t> mo;  ///< indices of MOs (this includes the sender and receiver)
		mo.push_back(0);
		for (size_t i = 0;;) {
			double max_angle = -std::numeric_limits<double>::infinity();
			size_t max_angle_index;

			for (size_t j = i+1; j < XY.size(); ++j) {
				double angle = (XY[j].second - XY[i].second) / (XY[j].first - XY[i].first);

				if (angle > max_angle) {
					max_angle = angle;
					max_angle_index = j;
				}
			}

			if (max_angle_index >= XY.size()-1) break;

			mo.push_back(max_angle_index);

			i = max_angle_index;
		}
		mo.push_back(XY.size()-1);

		// calculate attenuation due to MOs
		double attenuation_mo = 0;
		for (size_t mm=0; mm < mo.size()-2; ++mm) {
			size_t tx = mo[mm];
			size_t ob = mo[mm + 1];
			size_t rx = mo[mm + 2];

			double h1 = XY[tx].second;
			double h2 = XY[rx].second;
			double d = XY[rx].first - XY[tx].first;
			double d1 = XY[ob].first - XY[tx].first;
			double h = XY[ob].second;

			double ad_mo = getVehicleAttenuation(h1, h2, h, d, d1, f);

			attenuation_mo += ad_mo;
		}

		// calculate attenuation due to "small obstacles" (i.e. the ones in-between MOs)
		double attenuation_so = 0;
		for (size_t i=0; i < mo.size()-1; ++i) {
			size_t delta = mo[i+1] - mo[i];

			if (delta == 1) {
				// no obstacle in-between these two MOs
			}
			else if (delta == 2) {
				// one obstacle in-between these two MOs
				size_t tx = mo[i];
				size_t ob = mo[i] + 1;
				size_t rx = mo[i+1];

				double h1 = XY[tx].second;
				double h2 = XY[rx].second;
				double d = XY[rx].first - XY[tx].first;
				double d1 = XY[ob].first - XY[tx].first;
				double h = XY[ob].second;

				double ad_mo = getVehicleAttenuation(h1, h2, h, d, d1, f);
				attenuation_so += ad_mo;
			}
			else {
				// multiple obstacles in-between these two MOs -- use the one closest to their line of sight
				double x1 = XY[mo[i]].first;
				double y1 = XY[mo[i]].second;
				double x2 = XY[mo[i+1]].first;
				double y2 = XY[mo[i+1]].second;

				double min_delta_h = std::numeric_limits<float>::infinity();
				size_t min_delta_h_index;
				for (size_t j = mo[i]+1; j < mo[i+1]; ++j) {
					double h = (y2 - y1) / (x2 - x1) * (XY[j].first - x1) + y1;
					double delta_h = h - XY[j].second;

					if (delta_h < min_delta_h) {
						min_delta_h = delta_h;
						min_delta_h_index = j;
					}
				}

				size_t tx = mo[i];
				size_t ob = min_delta_h_index;
				size_t rx = mo[i+1];

				double h1 = XY[tx].second;
				double h2 = XY[rx].second;
				double d = XY[rx].first - XY[tx].first;
				double d1 = XY[ob].first - XY[tx].first;
				double h = XY[ob].second;

				double ad_mo = getVehicleAttenuation(h1, h2, h, d, d1, f);
				attenuation_so += ad_mo;
			}
		}

		double c;
		{
			double prodS = 1;
			double sumS = 0;
			double prodSsum = 1;
			double firstS = 0;
			double lastS = 0;

			double s_old = 0;
			for (size_t jj=0; jj < mo.size()-1; ++jj) {
				double s = XY[mo[jj+1]].first - XY[mo[jj]].first;  ///< distance between two MOs

				prodS *= s;
				sumS += s;
				if (jj == 0) firstS = s;
				else if (jj > 0) prodSsum *= (s + s_old);
				if (jj == mo.size()-2) lastS = s;
				s_old = s;
			}

			c = -10 * log10((prodS * sumS) / (prodSsum * firstS * lastS));
		}

		return attenuation_mo + attenuation_so + c;
	}
}

double ObstacleControl::calculateVehicleAttenuation(const Coord& senderPos, const Coord& receiverPos, const Signal& s) const {
	Enter_Method_Silent();

	const double not_a_number = std::numeric_limits<double>::quiet_NaN();
	double senderHeight = not_a_number;
	double receiverHeight = not_a_number;
	std::vector<std::pair<double, double> > potentialObstacles; /**< linear position of each obstructing vehicle along (senderPos--receiverPos) */

	simtime_t sStart = s.getSendingStart();

	EV << "searching candidates for transmission from " << senderPos.info() << " -> " << receiverPos.info() << " (" << senderPos.distance(receiverPos) << "meters total)" << std::endl;

	if (hasGUI() && annotations) {
		annotations->eraseAll(vehicleAnnotationGroup);
		drawVehicleObstacles(sStart);
		annotations->drawLine(senderPos, receiverPos, "blue", vehicleAnnotationGroup);
	}

	double x1 = std::min(senderPos.x, receiverPos.x);
	double x2 = std::max(senderPos.x, receiverPos.x);
	double y1 = std::min(senderPos.y, receiverPos.y);
	double y2 = std::max(senderPos.y, receiverPos.y);

	for (VehicleObstacles::const_iterator i = vehicleObstacles.begin(); i != vehicleObstacles.end(); ++i) {
		const TraCIMobility* m = (*i)->getTraCIMobility();
		Coord p  = m->getCurrentPosition();//PositionAt(sStart);
		double l = (*i)->getLength();
		double w = (*i)->getWidth();
		double h = (*i)->getHeight();

		EV << "checking vehicle at " << p.info() << " with height: " << h << " width: " << w << " length: " << l << endl;
		// shortcut if AABBs can't overlap
		double lw = std::max(l, w);
		if ((p.x + lw) < x1) continue;
		if ((p.x - lw) > x2) continue;
		if ((p.y + lw) < y1) continue;
		if ((p.y - lw) > y2) continue;

		if (p == senderPos) {
			// this is the sender
			senderHeight = h;
		}
		else if (p == receiverPos) {
			// this is the receiver
			receiverHeight = h;
		}
		else {
			// this is a potential obstacle
			double p1d = (*i)->getIntersectionPoint(senderPos, receiverPos);
			double maxd = senderPos.distance(receiverPos);
			if (!std::isnan(p1d) && p1d > 0 && p1d < maxd) {
				std::vector<std::pair<double, double> >::iterator it = potentialObstacles.begin();
				while (1) {
					if (it == potentialObstacles.end()) {
						potentialObstacles.push_back(std::make_pair(p1d, h));
						break;
					}
					if (it->first == p1d) { //omit double entries
						EV << "two obstacles at same distance " << it->first << " == " << p1d << " height: " << it->second << " =? " << h << std::endl;
						break;
					}
					if (it->first > p1d) {
						potentialObstacles.insert(it, std::make_pair(p1d, h));
						break;
					}
					++it;
				}
				EV << "\tgot obstacle in 2d-LOS at " << p.info() << ", " << p1d << " meters away from sender" << std::endl;
				Coord hitPos = senderPos + (receiverPos - senderPos) / senderPos.distance(receiverPos) * p1d;
				if (hasGUI() && annotations)
					annotations->drawLine(senderPos, hitPos, "red", vehicleAnnotationGroup);
			}
		}
	}

    //FIXME default heights for an RSU (because this is not in the set of vehicles or obstacles iterated over above)
    if (std::isnan(senderHeight)) {
      senderHeight = 5;
    }
    if (std::isnan(receiverHeight)) {
      receiverHeight = 5;
    }

	EV << "senderHeight: " << senderHeight << endl;
	EV << "receiverHeight: " << receiverHeight << endl;
	ASSERT(!std::isnan(senderHeight));
	ASSERT(!std::isnan(receiverHeight));
	if (std::isnan(senderHeight)) return 1;
	if (std::isnan(receiverHeight)) return 1;
	if (potentialObstacles.size() < 1) return 1;

	potentialObstacles.insert(potentialObstacles.begin(), std::make_pair(0, senderHeight));
	potentialObstacles.push_back(std::make_pair(senderPos.distance(receiverPos), receiverHeight));

	double attenuationDB = getVehicleAttenuation(potentialObstacles, carrierFrequency);

	EV << "t=" << simTime() << ": Attenuation by vehicles is " << attenuationDB << " dB" << std::endl;

	return pow(10.0, -attenuationDB/10.0);
}

double ObstacleControl::calculateAttenuation(const Coord& senderPos, const Coord& receiverPos) const {
	Enter_Method_Silent();

	if ((perCut.size() == 0) || (perMeter.size() == 0)) {
		throw cRuntimeError("Unable to use SimpleObstacleShadowing: No obstacle types have been configured");
	}
	if (obstacles.size() == 0) {
		throw cRuntimeError("Unable to use SimpleObstacleShadowing: No obstacles have been added");
	}

	// return cached result, if available
	CacheKey cacheKey(senderPos, receiverPos);
	CacheEntries::const_iterator cacheEntryIter = cacheEntries.find(cacheKey);
	if (cacheEntryIter != cacheEntries.end()) return cacheEntryIter->second;

	// calculate bounding box of transmission
	Coord bboxP1 = Coord(std::min(senderPos.x, receiverPos.x), std::min(senderPos.y, receiverPos.y));
	Coord bboxP2 = Coord(std::max(senderPos.x, receiverPos.x), std::max(senderPos.y, receiverPos.y));

	size_t fromRow = std::max(0, int(bboxP1.x / GRIDCELL_SIZE));
	size_t toRow = std::max(0, int(bboxP2.x / GRIDCELL_SIZE));
	size_t fromCol = std::max(0, int(bboxP1.y / GRIDCELL_SIZE));
	size_t toCol = std::max(0, int(bboxP2.y / GRIDCELL_SIZE));

	std::set<Obstacle*> processedObstacles;
	double factor = 1;
	for (size_t col = fromCol; col <= toCol; ++col) {
		if (col >= obstacles.size()) break;
		for (size_t row = fromRow; row <= toRow; ++row) {
			if (row >= obstacles[col].size()) break;
			const ObstacleGridCell& cell = (obstacles[col])[row];
			for (ObstacleGridCell::const_iterator k = cell.begin(); k != cell.end(); ++k) {

				Obstacle* o = *k;

				if (processedObstacles.find(o) != processedObstacles.end()) continue;
				processedObstacles.insert(o);

				// bail if bounding boxes cannot overlap
				if (o->getBboxP2().x < bboxP1.x) continue;
				if (o->getBboxP1().x > bboxP2.x) continue;
				if (o->getBboxP2().y < bboxP1.y) continue;
				if (o->getBboxP1().y > bboxP2.y) continue;

				double factorOld = factor;

				factor *= o->calculateAttenuation(senderPos, receiverPos);

				// draw a "hit!" bubble
				if (annotations && (factor != factorOld)) annotations->drawBubble(o->getBboxP1(), "hit");

				// bail if attenuation is already extremely high
				if (factor < 1e-30) break;

			}
		}
	}

	// cache result
	if (cacheEntries.size() >= 1000) cacheEntries.clear();
	cacheEntries[cacheKey] = factor;

	return factor;
}

double ObstacleControl::getAttenuationPerCut(std::string type) {
	if (perCut.find(type) != perCut.end()) return perCut[type];
	else {
		error("Obstacle type %s unknown", type.c_str());
		return -1;
	}
}

double ObstacleControl::getAttenuationPerMeter(std::string type) {
	if (perMeter.find(type) != perMeter.end()) return perMeter[type];
	else {
		error("Obstacle type %s unknown", type.c_str());
		return -1;
	}
}

bool ObstacleControl::isTypeSupported(std::string type) {
	//the type of obstacle is supported if there are attenuation values for borders and interior
	return (perCut.find(type) != perCut.end()) && (perMeter.find(type) != perMeter.end());
}

void ObstacleControl::drawVehicleObstacles(const simtime_t& t) const {
	for (VehicleObstacles::const_iterator i = vehicleObstacles.begin(); i != vehicleObstacles.end(); ++i) {
		VehicleObstacle* o = *i;
		annotations->drawPolygon(o->getShape(), "black", vehicleAnnotationGroup);
	}
}

void ObstacleControl::setCarrierFrequency(const double cf) {
  //TODO sanity checks
  carrierFrequency = cf;
}
