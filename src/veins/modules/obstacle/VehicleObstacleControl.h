//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include <list>

#include <omnetpp.h>
#include "veins/base/utils/Coord.h"
#include "veins/modules/obstacle/Obstacle.h"
#include "veins/modules/world/annotations/AnnotationManager.h"

#include "veins/base/utils/Move.h"
#include "veins/modules/obstacle/VehicleObstacle.h"
#include "veins/base/toolbox/Signal.h"

namespace Veins {

/**
 * VehicleObstacleControl models moving obstacles that block radio transmissions.
 *
 * Each Obstacle is a polygon.
 * Transmissions that cross one of the polygon's lines will have
 * their receive power set to zero.
 */
class VehicleObstacleControl : public cSimpleModule {
public:
    ~VehicleObstacleControl();
    void initialize(int stage);
    int numInitStages() const
    {
        return 2;
    }
    void finish();
    void handleMessage(cMessage* msg);
    void handleSelfMsg(cMessage* msg);

    const VehicleObstacle* add(VehicleObstacle obstacle);
    void erase(const VehicleObstacle* obstacle);

    /**
     * calculate attenuation factor that is due to vehicles
     */
    double calculateVehicleAttenuation(const Coord& senderPos, const Coord& receiverPos, const Signal& s) const;

    /**
     * Set carrier frequency
     */
    void setCarrierFrequency(const double frequency);

protected:
    bool debug; /**< whether to emit debug messages */

    AnnotationManager* annotations;
    double carrierFrequency;

    typedef std::list<VehicleObstacle*> VehicleObstacles;
    VehicleObstacles vehicleObstacles;
    AnnotationManager::Group* vehicleAnnotationGroup;
    void drawVehicleObstacles(const simtime_t& t) const;
};

class VehicleObstacleControlAccess {
public:
    VehicleObstacleControlAccess()
    {
    }

    VehicleObstacleControl* getIfExists()
    {
        return dynamic_cast<VehicleObstacleControl*>(getSimulation()->getModuleByPath("vehicleObstacles"));
    }
};

} // namespace Veins
