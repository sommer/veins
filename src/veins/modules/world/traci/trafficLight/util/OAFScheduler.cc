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
#include "veins/modules/world/traci/trafficLight/util/OAFScheduler.h"

using Veins::OAFScheduler;
using Veins::CAMRecord;
using Veins::VehicleRecord;

OAFScheduler::OAFScheduler():
    approachMeterPerSecond(3.0),
    scoreGainPerVehicle(0.2)
{}

OAFScheduler::OAFScheduler(const ConflictGraph& conflictGraph, double approachMeterPerSecond, double maxDistance,
                           double junctionProximityThreshold, simtime_t timeOut, double scoreGainPerVehicle):
    conflictGraph(conflictGraph),
    tracker(conflictGraph, timeOut, maxDistance, approachMeterPerSecond, junctionProximityThreshold),
    approachMeterPerSecond(approachMeterPerSecond),
    scoreGainPerVehicle(scoreGainPerVehicle)
{
}

OAFScheduler::approachPlatoonConfig OAFScheduler::scheduleNextPlatoons(simtime_t maxGreenTime) const
{
    // get arrival times for each approach
    std::map<approachId, simtime_t> firstArrivals = getEarliestArrivals();
    // get approach with earliest arrival time
    approachId earliestApproach = getEarliestArrivalApproach(firstArrivals);

    // collect approaches to be scheduled (earliest and non-conflicting)
    std::vector<approachId> selectedApproacheIds = {earliestApproach};
    std::set<approachId> nonConflictingApproachIds;
    for(auto&& itNonConflictingApproach : conflictGraph.getNonConflicts(earliestApproach)) {
        if(firstArrivals.find(itNonConflictingApproach.getId()) != firstArrivals.end() &&
            itNonConflictingApproach.getId() != earliestApproach) {
            nonConflictingApproachIds.insert(itNonConflictingApproach.getId());
        }
    }
    // if scheduled approaches have inferior approaches, schedule them, too!
    for(auto&& itInferiorId : conflictGraph.getApproach(earliestApproach).getInferiors()) {
        selectedApproacheIds.push_back(itInferiorId);
        nonConflictingApproachIds.erase(itInferiorId);
    }

    // append non-conflicting approaches to selectedApproachIds in order of ascending arrival time
    while(!nonConflictingApproachIds.empty()) {
        // select non-conflicting approach with earliest arrival
        approachId earliestNonConflictId = *std::min_element(
                                               nonConflictingApproachIds.begin(),
                                               nonConflictingApproachIds.end(),
        [&firstArrivals](const approachId &a, const approachId &b) {
            return firstArrivals.at(a) < firstArrivals.at(b);
        }
                                           );
        CompoundApproach earliestNonConflict(conflictGraph.getApproach(earliestNonConflictId));
        // add selected approach to list of selected approaches and erase them from the set
        selectedApproacheIds.push_back(earliestNonConflictId);
        nonConflictingApproachIds.erase(earliestNonConflictId);
        // also erase conflicts from list of non-conflicting approaches
        for (auto&& itConflict :earliestNonConflict.getConflicts()) {
            nonConflictingApproachIds.erase(itConflict);
        }
        // if the approach has inferior approaches, schedule them, too!
        for(auto&& itInferiorId : earliestNonConflict.getInferiors()) {
            selectedApproacheIds.push_back(itInferiorId);
            nonConflictingApproachIds.erase(itInferiorId);
        }
    }

    // get platoon configuration
    approachPlatoonConfig platoonConfig = computePlatoonConfiguration(maxGreenTime);
    approachPlatoonConfig scheduledPlatoons;
    for(auto&& itApproachId : selectedApproacheIds) {
        scheduledPlatoons[itApproachId] = platoonConfig[itApproachId];
    }
    return scheduledPlatoons;
}

OAFScheduler::greenTime OAFScheduler::estimateGreenTime(const approachId& approach, size_t platoonLength) const
{
    std::vector<VehicleRecord> appVehicles = tracker.vehiclesOnApproach(approach);
    // sort appVehicles
    // determine distance to junction for each element
    CompoundApproach&& approachObj = conflictGraph.getApproach(approach);
    std::map<VehicleId, double> distanceMap;
    for(auto&& itVehicle : appVehicles) {
        distanceMap[itVehicle.vehicleId] = approachObj.distanceToJunction(itVehicle.laneId, itVehicle.roadPosition);
    }
    std::sort(appVehicles.begin(), appVehicles.end(),
    [&distanceMap] (const CAMRecord& a, const CAMRecord& b) {
        return distanceMap[a.vehicleId] < distanceMap[b.vehicleId];
    });

    // decrease platoon length if vehicles are no longer available (e.g. switched lanes)
    platoonLength = std::min(platoonLength, appVehicles.size());
    CompoundApproach&& currentApproach = conflictGraph.getApproach(approach);

    // handle special cases
    if(platoonLength == 0) {
        return SimTime(0);
    } else if(platoonLength == 1) {
        double distance = currentApproach.distanceToJunction(appVehicles[0].laneId, appVehicles[0].roadPosition);
        double speed = std::max(appVehicles[0].speed / platoonLength, approachMeterPerSecond);
        return SimTime(1.5 + (distance / speed));
    }

    // extract relevant vehicles (on approach and in platoon interval)
    std::vector<CAMRecord> vehicles(appVehicles.begin(), std::next(appVehicles.begin(), platoonLength));

    // estimate platoon movement speed
    double speedSum = std::accumulate(vehicles.begin(), vehicles.end(), 0.0,
    [](double sum, const CAMRecord &item) {
        return sum + item.speed;
    });
    double avgSpeed = std::max(speedSum / platoonLength, approachMeterPerSecond);

    // compute distance to the junction for each vehicle to determine the distance in platoon
    double firstVehicleDistance = currentApproach.distanceToJunction(vehicles[0].laneId, vehicles[0].roadPosition);
    double lastVehicleDistance = currentApproach.distanceToJunction(vehicles[platoonLength-1].laneId,
                                 vehicles[platoonLength-1].roadPosition);

    // compute required green time
    return SimTime(1.5 + (lastVehicleDistance - firstVehicleDistance) / avgSpeed);
}

OAFScheduler::greenTime OAFScheduler::estimateMaxGreenTime(const approachPlatoonConfig& scheduledPlatoons) const
{
    greenTime highestGreenTime = 0;
    for(auto&& itConfig : scheduledPlatoons) {
        highestGreenTime = std::max(highestGreenTime, estimateGreenTime(itConfig.first, itConfig.second));
    }
    return highestGreenTime;
}

OAFScheduler::signalScheme OAFScheduler::makeSignalScheme(const approachPlatoonConfig& scheduledPlatoons) const
{
    ConflictGraph::approachSequence allApproaches(conflictGraph.getApproaches());

    signalScheme signal;
    for(auto&& itApproach : allApproaches) {
        if(scheduledPlatoons.find(itApproach.getId()) != scheduledPlatoons.end()) {
            signal.append("G");
        } else {
            signal.append("r");
        }
    }
    return signal;
}

std::vector<VehicleRecord> OAFScheduler::vehiclesOnApproach(const approachId& approach) const
{
    return tracker.vehiclesOnApproach(approach);
}

OAFScheduler::approachPlatoonConfig OAFScheduler::computePlatoonConfiguration(greenTime maxGreenTime) const
{
    if(tracker.empty()) {
        return approachPlatoonConfig();
    }

    // compute configurations for each (populated) approach
    // greenTimesConfigTable is essentially a table with approachIds as first axis (rows)
    // and platoon sizes as second axis (columns).
    // Each cell contains the green time for the platoon with the given size on the given approach.
    approachToConfigTable greenTimesConfigTable = computeConfigGreentimes(maxGreenTime);
    if(greenTimesConfigTable.empty()) {
        return approachPlatoonConfig();
    }

    // generate list of all available (=occupied) approaches and number of possible combinations
    std::vector<approachId> occupiedApproaches;
    unsigned int nrOfCombinations = 1;
    for(auto&& itApproachMap : greenTimesConfigTable) {
        occupiedApproaches.push_back(itApproachMap.first);
        nrOfCombinations *=  itApproachMap.second.size();
    }

    // find best configuration
    approachPlatoonConfig bestConfig; // store for the best known config so far
    std::map<approachId, std::map<platoon_t, greenTime>::iterator> approachToConfigItMap;
    for(auto&& approachConfig : greenTimesConfigTable) {
        approachToConfigItMap[approachConfig.first] = approachConfig.second.begin();
    }

    // check one configuration candidate
    // To do so, build "vertical paths" through the greenTimesConfigTable.
    // Start with one vehicle on each (populated) approach, i.e., fist cell in each row.
    // Then, in each further iteration, advance one row=(approach) by one column(=platton size) to the right.
    // If we advanced beyond the highest value for the column, reset it to the first cell
    // and try again to advance in the next row.
    for(unsigned int i = 0; i < nrOfCombinations; ++i) {
        std::vector<greenTime> greenTimes;
        approachPlatoonConfig config;
        bool doIncrement = i > 0; // do not increment on the very first configuration
        greenTime greenMax = 0;
        greenTime greenMin = maxGreenTime;
        platoon_t nrOfVehiclesInConfig = 0;
        double bestScore = -maxGreenTime.dbl(); // every score should be better than this

        // build configuration candidate to find max and min green times for all of its approaches.
        for(auto&& iteratorTuple : approachToConfigItMap) {
            // update index vector - advance to the next valid "vertical path" in the greenTimesConfigTable
            auto&& approachNr = iteratorTuple.first;
            auto&& targetIterator = iteratorTuple.second;
            if(doIncrement) {
                ++targetIterator;
                if(targetIterator == greenTimesConfigTable[approachNr].end()) {
                    // overflow - reset iterator and keep incrementing
                    targetIterator = greenTimesConfigTable[approachNr].begin();
                } else {
                    doIncrement = false;
                }
            }
            // build comparison data
            greenMax = std::max(greenMax, targetIterator->second);
            greenMin = std::min(greenMin, targetIterator->second);
            nrOfVehiclesInConfig += targetIterator->first;
        }

        // compare with previous best candidate
        double score = nrOfVehiclesInConfig * scoreGainPerVehicle - (greenMax - greenMin).dbl();
        if(score > bestScore) {
            bestScore = score;

            // build newly found best config
            bestConfig.clear();
            for(auto&& iteratorTuple : approachToConfigItMap) {
                auto&& approachNr = iteratorTuple.first;
                auto&& targetIterator = iteratorTuple.second;
                bestConfig[approachNr] = targetIterator->first;
            }
        }
    }
    return bestConfig;
}

OAFScheduler::approachToConfigTable OAFScheduler::computeConfigGreentimes(greenTime maxGreenTime) const
{
    approachToConfigTable result;
    for(auto&& approachName : tracker.getApproaches()) {
        auto vehiclesOnApproach = tracker.vehiclesOnApproach(approachName);
        unsigned int nrOfVehicles = vehiclesOnApproach.size();

        std::vector<platoon_t> configs;
        configs.reserve(nrOfVehicles);
        for(unsigned int i = 1; i <= nrOfVehicles; ++i)
            configs.push_back(i);

        std::map<platoon_t, greenTime> timesPerPlatoonSize;
        for(auto&& itPlatoonSize : configs) {
            greenTime curGreenTime = estimateGreenTime(approachName, itPlatoonSize);
            if(curGreenTime < maxGreenTime)
                timesPerPlatoonSize[itPlatoonSize] = curGreenTime;
        }
        if(!timesPerPlatoonSize.empty())
            result[approachName] = timesPerPlatoonSize;
    }
    return result;
}

OAFScheduler::platoon_t OAFScheduler::maxPlatoonSize(const approachPlatoonConfig& config) const
{
    platoon_t result = 0;
    for(auto&& itApproachPair : config) {
        result = std::max(result, itApproachPair.second);
    }
    return result;
}

std::map<OAFScheduler::approachId, simtime_t> OAFScheduler::getEarliestArrivals() const
{
    auto arrivalTimeComparator = [this](const VehicleRecord& a, const VehicleRecord& b) {
        return a.arrivalTime < b.arrivalTime;
    };
    std::map<approachId, simtime_t> firstArrivals;
    for(auto&& approachName : tracker.getApproaches()) {
        auto queue = tracker.vehiclesOnApproach(approachName);
       if(!queue.empty()) {
            auto firstVehicle = *std::min_element(queue.begin(), queue.end(), arrivalTimeComparator);
            firstArrivals[approachName] = firstVehicle.arrivalTime;
        }
    }
    return firstArrivals;
}

OAFScheduler::approachId OAFScheduler::getEarliestArrivalApproach(const approachArrivalMap& firstArrivals) const
{
    auto approachComparator = [](std::pair<approachId, simtime_t> a, std::pair<approachId, simtime_t> b) {
        return a.second < b.second;
    };
    return std::min_element(firstArrivals.begin(), firstArrivals.end(), approachComparator)->first;
}

