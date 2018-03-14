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
#include "veins/modules/world/traci/trafficLight/util/MonctonScheduler.h"

using Veins::MonctonScheduler;
using Veins::CAMRecord;

MonctonScheduler::MonctonScheduler():
    maxDistance(300.0),
    minWeight(1.0),
    maxWeight(40.0)
{}

MonctonScheduler::MonctonScheduler(const ConflictGraph& conflictGraph, const ScenarioSet& possibleScenarios,
                                   PhaseTime timeOut, double maxDistance, double decongestionFactor,
                                   double minWeight, double maxWeight):
    tracker(conflictGraph, timeOut),
    conflict_graph(conflictGraph),
    possibleScenarios(possibleScenarios),
    maxDistance(maxDistance),
    decongestionFactor(decongestionFactor),
    minWeight(minWeight),
    maxWeight(maxWeight)
{
    for(auto&& itScenario: possibleScenarios) {
        remainingScenarios.push_back(itScenario.first);
        for(auto&& itApproachId : itScenario.second) {
            approach2scenario[itApproachId] = itScenario.first;
        }
    }
}

bool MonctonScheduler::isPhaseToBeProlonged() const
{
    bool is_prolonged = false;
    for(auto&& itWeight : currentScenarioWeights) {
        if(itWeight.second * decongestionFactor < getApproachWeight(itWeight.first)) {
            is_prolonged = true;
            break;
        }
    }
    return is_prolonged;
}

unsigned int MonctonScheduler::scheduleNextPhaseNr()
{
    // gather approaches of remaining scenarios
    std::vector<ApproachId> remaining_approaches;
    for(auto&& itScenarioId : remainingScenarios) {
        remaining_approaches.insert(remaining_approaches.end(),
                                    possibleScenarios[itScenarioId].begin(),
                                    possibleScenarios[itScenarioId].end());
    }
    // determine approach weights
    std::map<ApproachId, double> approach_weights;
    for(auto&& itApproachId : remaining_approaches) {
        approach_weights[itApproachId] = getApproachWeight(itApproachId);
    }
    // select approach with highest weight
    auto selected_approach = std::max_element(
        approach_weights.begin(), approach_weights.end(),
        [] (const std::pair<ApproachId, double>& a, const std::pair<ApproachId, double>& b) {
            return a.second < b.second;
        }
    );
    // derive next scenario
    auto selected_scenarioId = approach2scenario[selected_approach->first];
    // store selected scenario's approaches' weights
    currentScenarioWeights.clear();
    for(auto&& itApproachId : possibleScenarios[selected_scenarioId]) {
        currentScenarioWeights[itApproachId] = approach_weights[itApproachId];
    }

    // remove selected scenario from pool of remaining scenarios
    remainingScenarios.erase(
        std::remove(remainingScenarios.begin(),
                    remainingScenarios.end(),
                    selected_scenarioId),
        remainingScenarios.end()
    );
    // if the pool of remaining scenarios is now empty, fill it up with all possible scenarios again
    if(remainingScenarios.empty())
        for(auto&& itScenario : possibleScenarios)
            remainingScenarios.push_back(itScenario.first);

    // derive and return phase number for the selected scenario
    return selected_scenarioId * 3; // times three because auf Green-Yellow-Red-Phases
}

double MonctonScheduler::getApproachWeight(const ApproachId& approach_id) const
{
    auto&& approach = conflict_graph.getApproach(approach_id);
    double weight = 0.0;
    for (auto&& itVehicle : tracker.vehiclesOnApproach(approach_id)) {
        double distance = approach.distanceToJunction(itVehicle.laneId, itVehicle.roadPosition);
        if (distance > maxDistance) {
            weight += minWeight;
        } else {
            weight += (1 - (distance / maxDistance)) * maxWeight;
        }
    }
    return weight;
}
