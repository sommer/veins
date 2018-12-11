//
// LinkControl - models Links that block radio transmissions
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

#ifndef LINK_LINKCONTROL_H
#define LINK_LINKCONTROL_H
#include <tuple>

#include <omnetpp.h>

#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/obstacle/Obstacle.h"
#include "../../BaseWaveApplLayer.h"

#include "float.h"
#include <omnetpp.h>
#include <veins/modules/application/f2mdVeinsApp/F2MDParameters.h>
#include <veins/modules/application/f2mdVeinsApp/mdSupport/NetworkLinksLib/Link.h>
#include "veins/base/utils/Coord.h"
#include <list>
#include <tuple>
#include <sstream>
#include <map>
#include <set>


/**
 * LinkControl models Links that block radio transmissions.
 *
 * Each Link is a polygon.
 * Transmissions that cross one of the polygon's lines will have
 * their receive power set to zero.
 */

using namespace Veins;
using namespace omnetpp;

class LinkControl {
public:
    ~LinkControl();
    void initialize(TraCICommandInterface* traci);

    void finish();
    void addShape(
            std::vector<Coord> shape);
    void add(Link Link);
    void erase(const Link* Link);

    /**
     * calculate additional attenuation by Links, return signal strength
     */
    double calculateDistance(const Coord& pos, double deltaX, double deltaY);

protected:
    struct CacheKey {
        const Coord senderPos;
        const Coord receiverPos;

        CacheKey(const Coord& senderPos, const Coord& receiverPos) :
                senderPos(senderPos), receiverPos(receiverPos) {
        }

        bool operator<(const CacheKey& o) const {
            if (senderPos.x < o.senderPos.x)
                return true;
            if (senderPos.x > o.senderPos.x)
                return false;
            if (senderPos.y < o.senderPos.y)
                return true;
            if (senderPos.y > o.senderPos.y)
                return false;
            if (receiverPos.x < o.receiverPos.x)
                return true;
            if (receiverPos.x > o.receiverPos.x)
                return false;
            if (receiverPos.y < o.receiverPos.y)
                return true;
            if (receiverPos.y > o.receiverPos.y)
                return false;
            return false;
        }
    };

    enum {
        GRIDCELL_SIZE = 32
    };

    typedef std::list<Link*> LinkGridCell;
    typedef std::vector<LinkGridCell> LinkGridRow;
    typedef std::vector<LinkGridRow> LinkList;
    typedef std::map<CacheKey, double> CacheEntries;

    LinkList Links;

    mutable CacheEntries cacheEntries;
};


#endif

