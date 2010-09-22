//
// ObstacleControl - models obstacles that block radio transmissions
// Copyright (C) 2006 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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


#ifndef OBSTACLE_OBSTACLECONTROL_H
#define OBSTACLE_OBSTACLECONTROL_H

#include <list>

#include <omnetpp.h>
#include "Coord.h"
#include "obstacle/Obstacle.h"

/**
 * ObstacleControl models obstacles that block radio transmissions.
 *
 * Each Obstacle is a polygon.
 * Transmissions that cross one of the polygon's lines will have
 * their receive power set to zero.
 */
class ObstacleControl : public cSimpleModule
{
	public:
		~ObstacleControl();
		void initialize();
		void finish();
		void handleMessage(cMessage *msg);
		void handleSelfMsg(cMessage *msg);
		void handleParameterChange(const char *parname);

		void addFromXml(cXMLElement* xml);
		void add(Obstacle obstacle);
		void erase(const Obstacle* obstacle);

		void draw(const Obstacle* obstacle);
		void expunge(const Obstacle* obstacle);
		void drawObstacles();
		void expungeObstacles();

		/**
		 * calculate additional attenuation by obstacles, return signal strength
		 */
		double calculateAttenuation(const Coord& senderPos, const Coord& receiverPos) const;

	protected:
		struct CacheKey {
			const Coord senderPos;
			const Coord receiverPos;

			CacheKey(const Coord& senderPos, const Coord& receiverPos) :
				senderPos(senderPos),
				receiverPos(receiverPos) {
			}

			bool operator<(const CacheKey& o) const {
				if (senderPos.getX() < o.senderPos.getX()) return true;
				if (senderPos.getX() > o.senderPos.getX()) return false;
				if (senderPos.getY() < o.senderPos.getY()) return true;
				if (senderPos.getY() > o.senderPos.getY()) return false;
				if (receiverPos.getX() < o.receiverPos.getX()) return true;
				if (receiverPos.getX() > o.receiverPos.getX()) return false;
				if (receiverPos.getY() < o.receiverPos.getY()) return true;
				if (receiverPos.getY() > o.receiverPos.getY()) return false;
				return false;
			}
		};

		enum { GRIDCELL_SIZE = 1024 };

		typedef std::list<Obstacle*> ObstacleGridCell;
		typedef std::vector<ObstacleGridCell> ObstacleGridRow;
		typedef std::vector<ObstacleGridRow> Obstacles;
		typedef std::map<const Obstacle*, std::list<cModule*> > DrawnObstacles;
		typedef std::map<CacheKey, double> CacheEntries;

		bool debug; /**< whether to emit debug messages */
		bool enabled; /**< when false all obstacles are ignored */
		cXMLElement* obstaclesXml; /**< obstacles to add at startup */

		Obstacles obstacles;
		DrawnObstacles drawnObstacles;
		mutable CacheEntries cacheEntries;
};

class ObstacleControlAccess
{
	public:
		ObstacleControlAccess() {
		}

		ObstacleControl* getIfExists() {
			return dynamic_cast<ObstacleControl*>(simulation.getModuleByPath("obstacles"));
		}
};

#endif

