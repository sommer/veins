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

#ifndef MONCTONSCHEDULER_H_
#define MONCTONSCHEDULER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "veins/modules/world/traci/trafficLight/util/CAMRecord.h"
#include "veins/modules/world/traci/trafficLight/util/ConflictGraph.h"
#include "veins/modules/world/traci/trafficLight/util/VehicleTracker.h"

namespace Veins {

class MonctonScheduler {
public:
    using ApproachId = CompoundApproach::approachId;
    using LaneId = CompoundApproach::laneId;
    using SignalScheme = std::string;
    using PhaseTime = simtime_t;
    using VehicleId = CAMRecord::VehicleId;
    using ApproachingVehicleQueue = std::list<VehicleId>;
    using Scenario = std::vector<ApproachId>;
    using ScenarioId = unsigned int;
    using ScenarioSet = std::map<ScenarioId, Scenario>;

    MonctonScheduler();
    MonctonScheduler(const ConflictGraph& conflictGraph, const ScenarioSet& possibleScenarios,
                     PhaseTime timeOut = SimTime(5.0), double maxDistance=300.0, double decongestionFactor=0.5,
                     double minWeight=1.0, double maxWeight=40.0);
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
    /*
     * shall the current phase be prolonged due remaining congestion?
     */
    bool isPhaseToBeProlonged() const;
    /*
     * return the next phase number to be scheduled
     */
    unsigned int scheduleNextPhaseNr();

private:
    /*
     * vehicle tracker to keep records of vehicles close to the junction
     */
    VehicleTracker tracker;
    /*
     * conflict graph object for this junction
     */
    ConflictGraph conflict_graph;
    /*
     * collection of all possible scenarios on this junction
     */
    ScenarioSet possibleScenarios;
    /*
     * list of scenarios still available in the current cycle
     */
    std::vector<ScenarioId> remainingScenarios;
    /*
     * maximum distance to consider vehicles for the junction
     */
    double maxDistance;
    /*
     * degree by which the weight of all green-served approaches has to be reduced to abort the green phase
     */
    double decongestionFactor;
    /*
     * minimum weight; for vehicles at maxDistance
     */
    double minWeight;
    /*
     * maximum weight; for vehicles directly next to the stopping line
     */
    double maxWeight;
    /*
     * initial weights of the currently active (green) scenario
     */
    std::map<ApproachId, double> currentScenarioWeights;
    /*
     * map each approach to the scenario in which it is serverd green light
     */
    std::map<ApproachId, ScenarioId> approach2scenario;

    /*
     * return the summed weight of all vehicles on the given approach
     */
    double getApproachWeight(const ApproachId& approach_id) const;
};

} /* namespace Veins */

#endif /* MONCTONSCHEDULER_H_ */
