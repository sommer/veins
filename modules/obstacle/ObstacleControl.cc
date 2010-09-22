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

#include "obstacle/ObstacleControl.h"


Define_Module(ObstacleControl);

ObstacleControl::~ObstacleControl() {

}

void ObstacleControl::initialize() {
	debug = par("debug");
	enabled = par("enabled");

	obstacles.clear();
	drawnObstacles.clear();
	cacheEntries.clear();

	obstaclesXml = par("obstacles");
	addFromXml(obstaclesXml);
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

void ObstacleControl::handleParameterChange(const char *parname) {
	if (parname && (std::string(parname) == "draw")) {
		if (ev.isGUI() && par("draw")) {
			drawObstacles();
		} else {
			expungeObstacles();
		}
	}
}

void ObstacleControl::addFromXml(cXMLElement* xml) {
	std::string rootTag = xml->getTagName();
	ASSERT (rootTag == "obstacles");

	cXMLElementList list = xml->getChildren();
	for (cXMLElementList::const_iterator i = list.begin(); i != list.end(); ++i) {
		cXMLElement* e = *i;

		std::string tag = e->getTagName();
		ASSERT(tag == "poly");

		// <poly id="building#0" type="building" color="#F00" shape="16,0 8,13.8564 -8,13.8564 -16,0 -8,-13.8564 8,-13.8564" />
		ASSERT(e->getAttribute("id"));
		std::string id = e->getAttribute("id");
		ASSERT(e->getAttribute("type"));
		std::string type = e->getAttribute("type");
		ASSERT(e->getAttribute("color"));
		std::string color = e->getAttribute("color");
		ASSERT(e->getAttribute("shape"));
		std::string shape = e->getAttribute("shape");

		double attenuationPerWall = 50; /**< in dB */
		double attenuationPerMeter = 1; /**< in dB / m */
		if (type == "building") { attenuationPerWall = 50; attenuationPerMeter = 1; }
		else error("unknown obstacle type: %s", type.c_str());
		Obstacle obs(id, attenuationPerWall, attenuationPerMeter);
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

}

void ObstacleControl::add(Obstacle obstacle) {
	Obstacle* o = new Obstacle(obstacle);

	size_t fromRow = std::max(0, int(o->getBboxP1().getX() / GRIDCELL_SIZE));
	size_t toRow = std::max(0, int(o->getBboxP2().getX() / GRIDCELL_SIZE));
	size_t fromCol = std::max(0, int(o->getBboxP1().getY() / GRIDCELL_SIZE));
	size_t toCol = std::max(0, int(o->getBboxP2().getY() / GRIDCELL_SIZE));
	for (size_t row = fromRow; row <= toRow; ++row) {
		for (size_t col = fromCol; col <= toCol; ++col) {
			if (obstacles.size() < col+1) obstacles.resize(col+1);
			if (obstacles[col].size() < row+1) obstacles[col].resize(row+1);
			(obstacles[col])[row].push_back(o);
		}
	}
	if (ev.isGUI() && par("draw")) draw(o);

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
	expunge(obstacle);
	delete obstacle;

	cacheEntries.clear();
}

void ObstacleControl::draw(const Obstacle* obstacle) {
	static int32_t nodeVectorIndex = -1;

	if (drawnObstacles.find(obstacle) != drawnObstacles.end()) return;

	ASSERT(obstacle->getShape().size() >= 2);
	Coord lastCoords = *obstacle->getShape().rbegin();
	for (std::vector<Coord>::const_iterator i = obstacle->getShape().begin(); i != obstacle->getShape().end(); ++i) {
		Coord c1 = *i;
		Coord c2 = lastCoords;
		lastCoords = c1;

		int w = abs(int(c2.getX()) - int(c1.getX()));
		int h = abs(int(c2.getY()) - int(c1.getY()));
		int px = 0;
		if (c1.getX() <= c2.getX()) {
			px = c1.getX() + 0.5*w;
		} else {
			px = c2.getX() + 0.5*w;
			w = -w;
		}
		int py = 0;
		if (c1.getY() <= c2.getY()) {
			py = c1.getY() + 0.5*h;
		} else {
			py = c2.getY() + 0.5*h;
			h = -h;
		}

		std::stringstream ss;
		ss << "p=" << px << "," << py << ";b=" << w << ", " << h << ",polygon,red,black,1";
		std::string displayString = ss.str();

		cModule* parentmod = getParentModule();
		if (!parentmod) error("Parent Module not found");

		cModuleType* nodeType = cModuleType::get("org.mixim.modules.obstacle.ObstacleDummy");

		//TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
		nodeVectorIndex++;
		cModule* mod = nodeType->create("obstacle", parentmod, nodeVectorIndex, nodeVectorIndex);
		mod->finalizeParameters();
		mod->getDisplayString().parse(displayString.c_str());
		mod->buildInside();
		mod->scheduleStart(simTime());

		drawnObstacles[obstacle].push_back(mod);
	}

}

void ObstacleControl::expunge(const Obstacle* obstacle) {
	if (drawnObstacles.find(obstacle) == drawnObstacles.end()) return;
	for (std::list<cModule*>::const_iterator i = drawnObstacles[obstacle].begin(); i != drawnObstacles[obstacle].end(); ++i) {
		cModule* mod = *i;
		mod->deleteModule();
	}
	drawnObstacles.erase(obstacle);
}

void ObstacleControl::drawObstacles() {
	for (Obstacles::const_iterator i = obstacles.begin(); i != obstacles.end(); ++i) {
		for (ObstacleGridRow::const_iterator j = i->begin(); j != i->end(); ++j) {
			for (ObstacleGridCell::const_iterator k = j->begin(); k != j->end(); ++k) {
				Obstacle* o = *k;
				draw(o);
			}
		}
	}
}

void ObstacleControl::expungeObstacles() {
	for (Obstacles::const_iterator i = obstacles.begin(); i != obstacles.end(); ++i) {
		for (ObstacleGridRow::const_iterator j = i->begin(); j != i->end(); ++j) {
			for (ObstacleGridCell::const_iterator k = j->begin(); k != j->end(); ++k) {
				Obstacle* o = *k;
				expunge(o);
			}
		}
	}
}

double ObstacleControl::calculateAttenuation(const Coord& senderPos, const Coord& receiverPos) const {
	Enter_Method_Silent();

	// bail if we're ignoring all obstacles
	if (!enabled) return 1;

	// return cached result, if available
	CacheKey cacheKey(senderPos, receiverPos);
	CacheEntries::const_iterator cacheEntryIter = cacheEntries.find(cacheKey);
	if (cacheEntryIter != cacheEntries.end()) return cacheEntryIter->second;

	// calculate bounding box of transmission
	Coord bboxP1 = Coord(std::min(senderPos.getX(), receiverPos.getX()), std::min(senderPos.getY(), receiverPos.getY()));
	Coord bboxP2 = Coord(std::max(senderPos.getX(), receiverPos.getX()), std::max(senderPos.getY(), receiverPos.getY()));

	bool draw = ev.isGUI() && par("draw");

	size_t fromRow = std::max(0, int(bboxP1.getX() / GRIDCELL_SIZE));
	size_t toRow = std::max(0, int(bboxP2.getX() / GRIDCELL_SIZE));
	size_t fromCol = std::max(0, int(bboxP1.getY() / GRIDCELL_SIZE));
	size_t toCol = std::max(0, int(bboxP2.getY() / GRIDCELL_SIZE));

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
				if (o->getBboxP2().getX() < bboxP1.getX()) continue;
				if (o->getBboxP1().getX() > bboxP2.getX()) continue;
				if (o->getBboxP2().getY() < bboxP1.getY()) continue;
				if (o->getBboxP1().getY() > bboxP2.getY()) continue;

				double factorOld = factor;

				factor *= o->calculateAttenuation(senderPos, receiverPos);

				// draw a "hit!" bubble
				if ((draw) && (factor != factorOld) && (drawnObstacles.find(o) != drawnObstacles.end())) {
					const std::list<cModule*>& mods = drawnObstacles.find(o)->second;
					if (mods.size() > 0) {
						cModule* mod = *mods.begin();
						mod->bubble("hit");
					}
				}

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
