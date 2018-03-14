/*
 * CAMRecord.h
 *
 *  Created on: Nov 24, 2015
 *      Author: dominik.buse
 */

#ifndef CAMRECORD_H_
#define CAMRECORD_H_

#include <string>
#include "veins/base/utils/Coord.h"
#include "veins/modules/messages/CooperativeAwarenessMessage_m.h"

namespace Veins {
struct CAMRecord {
    /**
     * type used to store an unique identifier for the corresponding vehicle
     */
    using VehicleId = std::string;
    using SenderId = uint32_t;

    VehicleId vehicleId;
    SenderId senderId;
    std::string edgeId;
    std::string laneId;
    int roadLaneNr;
    double roadPosition;
    Coord vehiclePosition;
    double angle;
    double speed;
    simtime_t timestamp;
};

bool operator==(const CAMRecord& a, const CAMRecord& b);

CAMRecord CAM2Record(CooperativeAwarenessMessage* cam);

} // end namespace



#endif /* CAMRECORD_H_ */
