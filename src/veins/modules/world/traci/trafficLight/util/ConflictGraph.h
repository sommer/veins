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


#ifndef CONFLICTGRAPH_H_
#define CONFLICTGRAPH_H_

#include <map>
#include <set>
#include <string>
#include <vector>
#include <omnetpp.h>

namespace Veins {

using omnetpp::cXMLElement;

class CompoundApproach {
public:
    using approachId = size_t; // type to identify an approach
    using laneId = std::string; // type to identify a lane (in SUMO)

    /**
     * simple constructor
     */
    CompoundApproach(approachId id=-1, std::vector<laneId> lanes= {}, std::vector<approachId> conflicts= {},
                     std::vector<approachId> inferiors= {});
    /**
     * constructor reading directly from an XML object
     */
    CompoundApproach(omnetpp::cXMLElement* xml);

    /**
     * return this approachs's id
     */
    approachId getId() const;
    /**
     * return key / characteristic trait entry (i.e. lane closest to the junction)
     */
    laneId getTrait() const;
    /**
     * return a vector of all lanes included in this approach sorted by proximity to the junction
     */
    std::vector<laneId> getLanes() const;

    /**
     * return the ids of all approaches in conflict with this one
     */
    std::vector<approachId> getConflicts() const;
    /**
     * return whether the other approach is in conflict with this one
     */
    bool isConflict(const approachId& other) const;
    /**
     * return the ids of all approaches inferior to this one
     */
    std::vector<approachId> getInferiors() const;
    /**
     * return the distance of from pos on lane to the junction (end of the first of approache's lanes) in meters
     */
    double distanceToJunction(const laneId& lane, double pos) const;

protected:

private:
    approachId id; /**< identifier of this approach */
    std::vector<laneId> lanes; /**< lanes forming up this approach sorted by proximity to the junction */
    std::vector<approachId> conflicts; /**< ids of approaches that are in conflict with this one*/
    std::vector<approachId> inferiors; /**< ids of approaches that are inferior to this one (dominated by it) */
    std::map<laneId, double> laneLengths; /**< length of each lane included in this vector */

    approachId readIdFromXML(cXMLElement* xml) const;
    std::vector<laneId> readLanesFromXml(cXMLElement* xml) const;
    std::vector<approachId> readConstraintsFromXML(cXMLElement* xml, const std::string& contraintTag) const;
    std::map<laneId, double> queryLaneLengths() const;
};

/**
 * less operator to make CompoundApproach usable for sorted containers
 */
bool operator<(const CompoundApproach a, const CompoundApproach b);

class ConflictGraph {
public:
    using approachId = CompoundApproach::approachId;
    using laneId = CompoundApproach::laneId;
    using approachSequence = std::vector<CompoundApproach>;

    ConflictGraph();
    ConflictGraph(cXMLElement* junction);

    /**
     * return all approaches in the conflict graph
     */
    approachSequence getApproaches() const;
    /**
     * return the approach with the given id
     */
    CompoundApproach getApproach(approachId id) const;
    /**
     * return the approach that contains the given lane
     */
    CompoundApproach getApproach(const laneId& lane) const;
    /**
     * return whether lane is present in any known approach
     */
    bool isLaneKnown(const laneId& lane) const;

    /**
     * return whether approach a and b are in conflict
     */
    bool isConflict(const approachId& a, const approachId& b) const;
    /**
     * return sequence of all elements in conflict with the given element
     */
    approachSequence getConflicts(const approachId& id) const;
    /**
     * return sequence of all elements NOT in conflict with the given element (includes element with given id itself)
     */
    approachSequence getNonConflicts(const approachId& id) const;

private:
    /**
     * main container of approaches within the graph
     */
    approachSequence approaches;
    /**
     * mapping from from lane identifier to approach
     */
    std::map<CompoundApproach::laneId,CompoundApproach> lane2approach;

    approachSequence readFromXML(cXMLElement* junction) const;
};

} // end namespace



#endif /* CONFLICTGRAPH_H_ */
