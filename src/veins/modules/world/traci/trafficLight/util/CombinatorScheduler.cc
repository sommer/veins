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

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <set>
#include <iterator>
#include "veins/modules/world/traci/trafficLight/util/CombinatorScheduler.h"

using Veins::CombinatorScheduler;
using Veins::CompoundApproach;
using Veins::ConflictGraph;
using Veins::VehicleRecord;
using PhaseTime = CombinatorScheduler::PhaseTime;
using PhaseSignal = CombinatorScheduler::PhaseSignal;
using ApproachId = CompoundApproach::approachId;
using ApproachCombination = std::set<ApproachId>;

namespace {
/*
 * create a dummy VehicleRecord for an imaginary vehicle at the very top of the approach
 */
VehicleRecord makeDummyLeader(const CompoundApproach& approach);
/*
 * compute current score/weight for scheduling of the specified vehicle
 */
double getVehicleScore(const VehicleRecord& vehicle, const VehicleRecord& leader, const CompoundApproach& approach);
/*
 * compute approach's score for all green phase dureations
 */
std::map<PhaseTime, double> getApproachScores(const CompoundApproach& approach, std::vector<VehicleRecord> vehicles,
        const std::vector<PhaseTime>& possibleGreenTimes);
/*
 * compute all valid (non-conflicting) combinations between approaches
 */
std::set<ApproachCombination> getCompatibleApproachCombinations(const std::set<CompoundApproach>& nonConflicts);
/*
 * build vector of green times, similar to python's range function
 */
std::vector<PhaseTime> makePossibleGreenTimes(const PhaseTime& tmin, const PhaseTime& tmax, const PhaseTime& tstep);

/*
 * TODO: implement/complete me
 * - add weights
 * - add more metrics
 */
double getVehicleScore(const VehicleRecord& vehicle, const VehicleRecord& leader, const CompoundApproach& approach)
{
    // distance to leader (in meters)
    double stopLineDistance = approach.distanceToJunction(vehicle.laneId, vehicle.roadPosition);
    double stopLineDistanceLeader = approach.distanceToJunction(leader.laneId, leader.roadPosition);
    double distanceToLeader = stopLineDistance - stopLineDistanceLeader;
    double scoreDistanceToLeader = 1.0 / distanceToLeader;

    // current speed
    double scoreCurrentSpeed = vehicle.speed;

    return scoreDistanceToLeader + scoreCurrentSpeed;
}

std::map<PhaseTime, double> getApproachScores(const CompoundApproach& approach, std::vector<VehicleRecord> vehicles,
        const std::vector<PhaseTime>& possibleGreenTimes)
{
    std::map<PhaseTime, double> result;
    double score = 0.0;
    if(approach.getLanes().empty()) {
        // there might be approaches with zero lanes
        // they cannot have phases on their own but must be inferior to some other approach with at least one lane
        return result;
    }

    VehicleRecord dummyLeader = makeDummyLeader(approach);
    vehicles.insert(vehicles.begin(), dummyLeader);
    auto vehIt = std::next(vehicles.begin());
    for(PhaseTime t : possibleGreenTimes) {
        // find vehicles that can pass before t
        for(;vehIt != vehicles.end(); ++vehIt) {
            if(vehIt->arrivalTime > t) {
                break;
            }
            // compute vehicles score and accumulate timeslot score
            score += getVehicleScore(*vehIt, *std::prev(vehIt), approach);
        }
        result[t] = score;
    }
    return result;
}

VehicleRecord makeDummyLeader(const CompoundApproach& approach)
{
    VehicleRecord dummyLeader;
    dummyLeader.timestamp = simTime();
    dummyLeader.arrivalTime = simTime();
    dummyLeader.laneId = approach.getLanes().front();
    dummyLeader.roadPosition = 0.0;
    return dummyLeader;
}

std::set<ApproachCombination> getCompatibleApproachCombinations(const std::set<CompoundApproach>& nonConflicts)
{
    std::set<ApproachCombination> result;
    if(nonConflicts.empty()) {
        return result;
    }

    for(auto&& itNonConflict : nonConflicts) {
        auto itNonConflictId = itNonConflict.getId();
        // find remaining non-conflicting approaches
        std::set<CompoundApproach> remainingNonConflicts = {};
        std::copy_if(nonConflicts.begin(), nonConflicts.end(),
                std::inserter(remainingNonConflicts, remainingNonConflicts.begin()),
                [&itNonConflict, &itNonConflictId] (const CompoundApproach& other) {
                    return other.getId() != itNonConflictId && !itNonConflict.isConflict(other.getId());
        });
        // recurse to get solutions for itNonConflict
        auto subResults = getCompatibleApproachCombinations(remainingNonConflicts);
        // insert selected Id into all sub-solutions
        for(auto itSubResult : subResults) {
            itSubResult.insert(itNonConflictId);
            result.insert(itSubResult);
        }
        // insert selected Id directly
        result.insert({itNonConflictId});
    }
    return result;
}

std::set<ApproachCombination> getAllCompatibleApproachCombinations(const ConflictGraph& conflict_graph)
{
    std::set<CompoundApproach> as_set;
    auto&& all_approaches = conflict_graph.getApproaches();
    std::copy(all_approaches.begin(), all_approaches.end(), std::inserter(as_set, as_set.begin()));
    return getCompatibleApproachCombinations(as_set);
}

std::vector<PhaseTime> makePossibleGreenTimes(const PhaseTime& tmin, const PhaseTime& tmax, const PhaseTime& tstep)
{
    std::vector<PhaseTime> result;
    for(PhaseTime t = tmin; t <= tmax; t += tstep)
        result.push_back(t);
    return result;
}

} // namespace ""


CombinatorScheduler::CombinatorScheduler()
{}

CombinatorScheduler::CombinatorScheduler(const ConflictGraph& conflictGraph, const PhaseTime& green_min,
        const PhaseTime& green_max, const PhaseTime& green_step, PhaseTime timeOut, double maxDistance):
    tracker(conflictGraph, timeOut, maxDistance),
    conflict_graph(conflictGraph),
    approachCombinations(getAllCompatibleApproachCombinations(conflictGraph)),
    possibleGreenTimes(makePossibleGreenTimes(green_min, green_max, green_step))
{}

std::pair<CombinatorScheduler::PhaseSignal, CombinatorScheduler::PhaseTime> CombinatorScheduler::makeNextPhase() const
{
    // compute score rows for each approach
    std::map<ApproachId, std::map<PhaseTime, double>> approachTimeScoreTable;
    for(auto itApproach : conflict_graph.getApproaches()) {
        auto approachId = itApproach.getId();
        approachTimeScoreTable[approachId] = getApproachScores(itApproach, tracker.vehiclesOnApproach(approachId),
                possibleGreenTimes);
    }
    // construct combination table
    // NOTE: this could also be done without constructing the whole table in memory first
    //       however, for debugging purposes this is more and easily comprehensable
    std::map<std::pair<ApproachCombination, PhaseTime>, double> approachCombinationScoreTable;
    for(auto&& itCombination : approachCombinations) {
        for(auto&& itTime : possibleGreenTimes) {
            double summedScore = 0.0;
            for(auto&& itApproachId : itCombination)
                summedScore += approachTimeScoreTable[itApproachId][itTime];
            approachCombinationScoreTable[std::make_pair(itCombination, itTime)] = summedScore;
        }
    }
    // select next phase configuration -> select cell with highest score
    auto itBest = std::max_element(approachCombinationScoreTable.cbegin(), approachCombinationScoreTable.cend(),
            [](const std::pair<std::pair<ApproachCombination, PhaseTime>, double>& lhs,
               const std::pair<std::pair<ApproachCombination, PhaseTime>, double>& rhs)
              { return lhs.second < rhs.second; }
    );
    // map selected phase configuration to a signalling scheme
    PhaseSignal signal;
    auto selectedConfig = itBest->first.first;
    for(auto&& itApproach : conflict_graph.getApproaches()) {
        if(selectedConfig.find(itApproach.getId()) != selectedConfig.end()) {
            signal.append("G");
        } else {
            signal.append("r");
        }
    }
    // compose result
    return std::make_pair(signal, itBest->first.second);
}
