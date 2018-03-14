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

#ifndef COMBINATORSCHEDULER_H_
#define COMBINATORSCHEDULER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "veins/modules/world/traci/trafficLight/util/ConflictGraph.h"
#include "veins/modules/world/traci/trafficLight/util/VehicleTracker.h"

namespace Veins {

class CombinatorScheduler {
public:
    using PhaseTime = simtime_t;
    using PhaseSignal = std::string;

    CombinatorScheduler();
    CombinatorScheduler(const ConflictGraph& conflictGraph, const PhaseTime& green_min, const PhaseTime& green_max,
            const PhaseTime& green_step, PhaseTime timeOut=SimTime(5.0), double maxDistance=300.0);
    /*
     * pass a vehicle to the scheduler and return whether it is (still) relevant
     */
    bool handleVehicle(const CAMRecord& vehicleRecord) { return tracker.handleVehicle(vehicleRecord); }
    /*
     * return whether this scheduler currently manages any vehicles
     */
    bool empty() const { return tracker.empty(); }
    /*
     * remove all records of vehicles that have not been updated for some time
     */
    void clearTimedOutVehicles() { tracker.clearTimedOutVehicles(); }
    /*
     * remove a vehicle from the tracker by id
     */
    void clearVehicle(const VehicleRecord::VehicleId& vehId) { tracker.clearVehicle(vehId); }
    /*
     * compute next phase signals and duration based on current vehicle information
     */
    std::pair<PhaseSignal, PhaseTime> makeNextPhase() const;

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
     * set of all valid (non-conflicting) approach combinations
     *
     * each approach combination is implemented as another set (of approach ids)
     */
    std::set<std::set<CompoundApproach::approachId>> approachCombinations;
    /*
     * sequence of all possible green times considered by this scheduler
     */
    std::vector<PhaseTime> possibleGreenTimes;
};

} /* namespace Veins */

#endif /* COMBINATORSCHEDULER_H_ */
