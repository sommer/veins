//
// Copyright (C) 2017 Dominik Buse <dbuse@mail.uni-paderborn.de>
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

#include <algorithm>

#include "veins/modules/world/traci/trafficLight/util/VehicleTracker.h"

using Veins::CAMRecord;
using Veins::VehicleRecord;
using Veins::VehicleTracker;

VehicleTracker::VehicleTracker():
    maxDistance(300.0),
    timeOut(SimTime(5.0)),
    approachMeterPerSecond(3.0),
    junctionProximityThreshold(5.0)
{
}

VehicleTracker::VehicleTracker(const ConflictGraph conflictGraph,
                   const simtime_t timeOut, double maxDistance,
                   double junctionProximityThreshold, double approachMeterPerSecond):
    conflictGraph(conflictGraph),
    maxDistance(maxDistance),
    timeOut(timeOut),
    approachMeterPerSecond(approachMeterPerSecond),
    junctionProximityThreshold(junctionProximityThreshold)
{
    for(auto&& itApproach : conflictGraph.getApproaches()) {
        approachingVehicles[itApproach.getId()] = VehicleQueue();
    }
}

bool VehicleTracker::handleVehicle(const CAMRecord& vehicleRecord)
{
    bool is_relevant = isRelevant(vehicleRecord);
    bool is_known = isKnown(vehicleRecord.vehicleId);

    if(is_relevant && !is_known) {
        registerVehicle(vehicleRecord);
    } else if(is_relevant && is_known) {
        updateVehicle(vehicleRecord);
    } else if(is_known && !is_relevant) {
        unregisterVehicle(vehicleRecord.vehicleId);
    }
    return is_relevant;
}

bool VehicleTracker::empty() const
{
    return knownVehicles.empty();
}

void VehicleTracker::clearTimedOutVehicles()
{
    simtime_t clearBefore {simTime() - timeOut};
    // determine list of timed out vehicles
    std::vector<CAMRecord> timedOutVehicles;
    for(auto&& itKnownVehicle : knownVehicles) {
        if(itKnownVehicle.second.timestamp < clearBefore) {
            timedOutVehicles.push_back(itKnownVehicle.second);
        }
    }
    // unregister them
    for(auto&& timedOutVehicle : timedOutVehicles) {
        unregisterVehicle(timedOutVehicle.vehicleId);
    }
}

void VehicleTracker::clearVehicle(const VehicleId& vehId)
{
    unregisterVehicle(vehId);
}

VehicleRecord VehicleTracker::getVehicle(const VehicleId vehicleId) const
{
    ASSERT(knownVehicles.find(vehicleId) != knownVehicles.end());
    return knownVehicles.at(vehicleId);
}

std::vector<VehicleTracker::ApproachId> VehicleTracker::getApproaches() const
{
    std::vector<ApproachId> result;
    result.reserve(approachingVehicles.size());
    for(auto&& itPair : approachingVehicles)
        result.push_back(itPair.first);
    return result;
}

void VehicleTracker::registerVehicle(const CAMRecord& vehicleRecord)
{
    VehicleQueue& queue = approachingVehicles.at(conflictGraph.getApproach(vehicleRecord.laneId).getId());
    auto arrivalTime = estimateArrivalTime(vehicleRecord);
    knownVehicles[vehicleRecord.vehicleId] = VehicleRecord(vehicleRecord, arrivalTime, simTime());
    queue.push_back(vehicleRecord.vehicleId);
}

void VehicleTracker::unregisterVehicle(const VehicleId& vehId)
{
    const CompoundApproach::laneId lastSeenLane = knownVehicles.at(vehId).laneId;
    const ApproachId lastApproach = conflictGraph.getApproach(lastSeenLane).getId();
    VehicleQueue& queue =  approachingVehicles.at(lastApproach);

    queue.erase(std::find(queue.begin(), queue.end(), vehId));
    knownVehicles.erase(vehId);
}

void VehicleTracker::updateVehicle(const CAMRecord& vehicleRecord)
{
    auto oldApproachId = conflictGraph.getApproach(knownVehicles.at(vehicleRecord.vehicleId).laneId).getId();
    auto newApproachId = conflictGraph.getApproach(vehicleRecord.laneId).getId();

    if(oldApproachId != newApproachId) { // no longer on the same lane - update the queue
        VehicleQueue& oldQueue = approachingVehicles.at(oldApproachId);
        VehicleQueue& newQueue = approachingVehicles.at(newApproachId);
        newQueue.push_back(vehicleRecord.vehicleId);
        oldQueue.erase(std::find(oldQueue.begin(), oldQueue.end(), vehicleRecord.vehicleId));
    }
    // in any case - update the record
    knownVehicles.at(vehicleRecord.vehicleId) = VehicleRecord(vehicleRecord, estimateArrivalTime(vehicleRecord), simTime());
}

bool VehicleTracker::isRelevant(const CAMRecord& vehicleRecord) const
{
    if(!conflictGraph.isLaneKnown(vehicleRecord.laneId))
        return false; // lane not part of junction

    double distance = conflictGraph.getApproach(vehicleRecord.laneId).distanceToJunction(
            vehicleRecord.laneId, vehicleRecord.roadPosition);
    return distance <= maxDistance;
}

bool VehicleTracker::isKnown(const VehicleId& vehId) const
{
    return knownVehicles.find(vehId) != knownVehicles.end();
}

std::vector<VehicleRecord> VehicleTracker::vehiclesOnApproach(const ApproachId& approach) const
{
    std::vector<VehicleRecord> result;
    ASSERT(approachingVehicles.find(approach) != approachingVehicles.end());
    for(auto&& vehId : approachingVehicles.at(approach)) {
        ASSERT(knownVehicles.find(vehId) != knownVehicles.end());
        result.push_back(knownVehicles.at(vehId));
    }
    return result;
}

simtime_t VehicleTracker::estimateArrivalTime(const CAMRecord& vehicleRecord) const
{
    auto&& currentApproach = conflictGraph.getApproach(vehicleRecord.laneId);
    double distanceToJunction = currentApproach.distanceToJunction(vehicleRecord.laneId, vehicleRecord.roadPosition);

    if(distanceToJunction < junctionProximityThreshold) {
        // vehicle is at the stop line - arrivalTime must not increase from here on
        if(!isKnown(vehicleRecord.vehicleId))
            return vehicleRecord.timestamp; // no previously known value (not very probable but possible)
        ASSERT(isKnown(vehicleRecord.vehicleId));
        return knownVehicles.at(vehicleRecord.vehicleId).arrivalTime;
    } else {
        // vehicle is moving (towards the stop line) - arrival time is in the future
        double speed = std::max(vehicleRecord.speed, approachMeterPerSecond);
        return vehicleRecord.timestamp + SimTime(distanceToJunction / speed);
    }
}
