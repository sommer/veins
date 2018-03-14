//
// Copyright (C) 2015 Dominik Buse <dbuse@mail.uni-paderborn.de>
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

#ifndef OAFSCHEDULER_H_
#define OAFSCHEDULER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "veins/modules/world/traci/trafficLight/util/CAMRecord.h"
#include "veins/modules/world/traci/trafficLight/util/ConflictGraph.h"
#include "veins/modules/world/traci/trafficLight/util/VehicleTracker.h"

namespace Veins {

class OAFScheduler {
public:
    using approachId = CompoundApproach::approachId;
    using laneId = CompoundApproach::laneId;
    using signalScheme = std::string;
    using greenTime = simtime_t;
    using VehicleId = CAMRecord::VehicleId;
    using platoon_t = unsigned int;
    using approachToConfigTable = std::map<approachId, std::map<platoon_t, greenTime>>;
    using approachPlatoonConfig = std::map<approachId,platoon_t>;
    using approachArrivalMap = std::map<approachId, simtime_t>;

    OAFScheduler();
    OAFScheduler(const ConflictGraph& conflictGraph, double approachMeterPerSecond = 3.0, double maxDistance = 300.0,
                 double junctionProximityThreshold = 5.0, simtime_t timeOut = SimTime(5.0), double scorefactor=0.2);
    /**
     * return a schedule using the OAF algorithm
     */
    approachPlatoonConfig scheduleNextPlatoons(greenTime maxGreenTime) const;
    /**
     * register a vehicle to be considered in the next scheduler run and return whether it is (still) relevant
     */
    bool handleVehicle(const CAMRecord& vehicleRecord) { return tracker.handleVehicle(vehicleRecord); }
    /**
     * return whether this scheduler currently manages any vehicles
     */
    bool empty() const { return tracker.empty(); }
    /**
     * remove all records of vehicles that have not been updated for some time
     */
    void clearTimedOutVehicles() { tracker.clearTimedOutVehicles(); }
    /*
     * remove a vehicle from the tracker by id
     */
    void clearVehicle(const VehicleId& vehId) { tracker.clearVehicle(vehId); }
    /**
     * return estimation of the required green time for the platoon configuration on the given approach
     */
    greenTime estimateGreenTime(const approachId& approach, size_t platoonLength) const;
    /**
     * return the estimated maximum green time required for the given platoon configuration
     */
    greenTime estimateMaxGreenTime(const approachPlatoonConfig& scheduledPlatoons) const;
    /**
     * return the signal scheme for the given (scheduled) approaches
     */
    signalScheme makeSignalScheme(const approachPlatoonConfig& scheduledPlatoons) const;
    /*
     * return list of vehicle records on the approach
     */
    std::vector<VehicleRecord> vehiclesOnApproach(const approachId& approach) const;

private:
    /*
     * conflict graph object for this junction
     */
    ConflictGraph conflictGraph;
    /*
     * vehicle tracker to keep records of vehicles close to the junction
     */
    VehicleTracker tracker;
    /*
     * average speed at which vehicles move when approaching a junction
     */
    double approachMeterPerSecond;
    /*
     * factor to be multiplied by the number of vehicles when determining configuration score
     *
     * unit is roughly meter/vehicle
     */
    double scoreGainPerVehicle;

    /**
     * return final/ideal platoon configuration
     *
     * implements the platooning Algorithm
     */
    approachPlatoonConfig computePlatoonConfiguration(greenTime maxGreenTime) const;
    /**
     * compute currently possible platoon configurations (for each approach) and their green times
     */
    approachToConfigTable computeConfigGreentimes(greenTime maxGreenTime) const;
    /**
     * return maximum platoon size of given platoon configuration
     */
    platoon_t maxPlatoonSize(const approachPlatoonConfig& config) const;
    /**
     * return mapping from each approach to its earliest vehicles arrival time
     */
    std::map<approachId, simtime_t> getEarliestArrivals() const;
    /**
     * return name of the approach with the earliest arrival time among all (currently occupied) approaches
     */
    approachId getEarliestArrivalApproach(const std::map<approachId, simtime_t>& firstArrivals) const;
};

} /* namespace Veins */

#endif /* OAFSCHEDULER_H_ */
