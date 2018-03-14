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
#include "veins/modules/world/traci/trafficLight/util/ConflictGraph.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

using Veins::ConflictGraph;
using Veins::CompoundApproach;

CompoundApproach::CompoundApproach(approachId id, std::vector<laneId> lanes, std::vector<approachId> conflicts,
                                   std::vector<approachId> inferiors):
    id(id),
    lanes(lanes),
    conflicts(conflicts),
    inferiors(inferiors),
    laneLengths(queryLaneLengths())
{
}

CompoundApproach::CompoundApproach(cXMLElement* xml):
    id(readIdFromXML(xml)),
    lanes(readLanesFromXml(xml)),
    conflicts(readConstraintsFromXML(xml, "conflict")),
    inferiors(readConstraintsFromXML(xml, "inferior")),
    laneLengths(queryLaneLengths())
{
}

CompoundApproach::approachId CompoundApproach::getId() const
{
    return id;
}

CompoundApproach::laneId CompoundApproach::getTrait() const
{
    return lanes.front();
}

std::vector<CompoundApproach::laneId> CompoundApproach::getLanes() const
{
    return lanes;
}

std::vector<CompoundApproach::approachId> CompoundApproach::getConflicts() const
{
    return conflicts;
}

bool CompoundApproach::isConflict(const approachId& other) const
{
    return std::find(conflicts.begin(), conflicts.end(), other) != conflicts.end();
}

std::vector<CompoundApproach::approachId> CompoundApproach::getInferiors() const
{
    return inferiors;
}

double CompoundApproach::distanceToJunction(const laneId& lane, double pos) const
{
    double distance = 0;
    for(auto&& itLane : lanes) {
        distance += laneLengths.at(itLane);
        if(itLane == lane) break;
    }
    return distance - pos;
}

CompoundApproach::approachId CompoundApproach::readIdFromXML(cXMLElement* xml) const
{
    return std::stoi(xml->getAttribute("id"));
}

std::vector<CompoundApproach::laneId> CompoundApproach::readLanesFromXml(cXMLElement* xml) const
{
    std::vector<laneId> result;
    auto xmlLanes = xml->getChildrenByTagName("lane");
    for(auto&& itLane : xmlLanes) {
        result.push_back(itLane->getAttribute("id"));
    }
    return result;
}

std::vector<CompoundApproach::approachId> CompoundApproach::readConstraintsFromXML(cXMLElement* xml,
        const std::string& contraintTag) const
{
    std::vector<approachId> result;
    for(auto&& itConstraint : xml->getChildrenByTagName(contraintTag.c_str())) {
        result.push_back(std::stoi(itConstraint->getAttribute("other")));
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::map<CompoundApproach::laneId, double> CompoundApproach::queryLaneLengths() const
{
    std::map<laneId, double> result;
    auto&& iface(TraCIScenarioManagerAccess().get()->getCommandInterface());
    for(auto&& itLane : lanes) {
        result[itLane] = iface->lane(itLane).getLength();
    }
    return result;
}

bool Veins::operator<(const CompoundApproach a, const CompoundApproach b)
{
    return a.getId() < b.getId();
}

ConflictGraph::ConflictGraph()
{
}

ConflictGraph::ConflictGraph(cXMLElement* junction):
    approaches(readFromXML(junction->getFirstChildWithTag("approaches")))
{
    for(auto&& itApproach : approaches) {
        for(auto&& itLane : itApproach.getLanes()) {
            lane2approach[itLane] = itApproach;
        }
    }
}

ConflictGraph::approachSequence ConflictGraph::getApproaches() const
{
    return approaches;
}

CompoundApproach ConflictGraph::getApproach(ConflictGraph::approachId id) const
{
    return approaches.at(id);
}

CompoundApproach ConflictGraph::getApproach(const ConflictGraph::laneId& lane) const
{
    return lane2approach.at(lane);
}

bool ConflictGraph::isLaneKnown(const ConflictGraph::laneId& lane) const
{
    return lane2approach.find(lane) != lane2approach.end();
}

bool ConflictGraph::isConflict(const approachId& a, const approachId& b) const
{
    return approaches.at(a).isConflict(b);
}

ConflictGraph::approachSequence ConflictGraph::getConflicts(const approachId& id) const
{
    std::vector<approachId> conflict_ids{approaches.at(id).getConflicts()};
    approachSequence conflicts;
    for(auto&& itConflictId : conflict_ids) {
        conflicts.push_back(approaches.at(itConflictId));
    }
    return conflicts;
}

ConflictGraph::approachSequence ConflictGraph::getNonConflicts(const approachId& id) const
{
    approachSequence conflicts = getConflicts(id);
    approachSequence non_conflicts;
    std::set_difference(approaches.begin(), approaches.end(), conflicts.begin(), conflicts.end(),
                        std::back_inserter(non_conflicts),
    [](const CompoundApproach& a, const CompoundApproach& b) {
        return a.getId() < b.getId();
    });
    return non_conflicts;
}

// Private Methods of ConflictGraph
std::vector<CompoundApproach> ConflictGraph::readFromXML(cXMLElement* junction) const
{
    auto xml_children = junction->getChildrenByTagName("approach");
    std::vector<CompoundApproach> result(xml_children.size());

    for(auto&& itApproach : xml_children) {
        CompoundApproach approach(itApproach);
        result[approach.getId()] = approach;
    }
    // make sure the approaches are sorted (by id using overloaded less operator)
    std::sort(result.begin(), result.end());

    return result;
}
