/*
 * CAMRecord.cc
 *
 *  Created on: Nov 24, 2015
 *      Author: dominik.buse
 */

#include "veins/modules/world/traci/trafficLight/util/CAMRecord.h"

namespace Veins {

bool operator==(const Veins::CAMRecord& a, const Veins::CAMRecord& b)
{
    return a.timestamp == b.timestamp && a.vehicleId == b.vehicleId;
}

CAMRecord CAM2Record(CooperativeAwarenessMessage* cam)
{
    CAMRecord record;
    record.vehicleId = cam->getVehicleId();
    record.senderId = cam->getSenderAddress();
    record.edgeId = cam->getCurrentEdge();
    record.roadLaneNr = cam->getCurrentLandNr();
    record.laneId = record.edgeId + "_" + std::to_string(record.roadLaneNr);
    record.roadPosition = cam->getCurrentLanePosition();
    record.vehiclePosition = cam->getSenderPos();
    record.speed = cam->getSpeed();
    record.angle = cam->getAngle();
    record.timestamp = cam->getTimestamp();
    return record;
}

}
