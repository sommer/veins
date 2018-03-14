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

#ifndef VEHICLETRACKER_H_
#define VEHICLETRACKER_H_

#include <vector>
#include <map>

#include <omnetpp.h>
#include "veins/modules/world/traci/trafficLight/util/CAMRecord.h"
#include "veins/modules/world/traci/trafficLight/util/ConflictGraph.h"

namespace Veins {

struct VehicleRecord: public CAMRecord {
public:
    simtime_t arrivalTime;
    simtime_t timestamp;

    VehicleRecord():
        CAMRecord(),
        arrivalTime(SimTime(-1)),
        timestamp(SimTime(0))
    {}

    VehicleRecord(const CAMRecord& camRecord, simtime_t arrivalTime, simtime_t timestamp):
        CAMRecord(camRecord),
        arrivalTime(arrivalTime),
        timestamp(timestamp)
    {}
};

class VehicleTracker {
public:
    using ApproachId = CompoundApproach::approachId;
    using VehicleId = CAMRecord::VehicleId;
    using VehicleQueue = std::vector<VehicleId>;

    VehicleTracker();
    VehicleTracker(const ConflictGraph conflictGraph,
                   const simtime_t timeOut = SimTime(5.0), double maxDistance=300.0,
                   double junctionProximityThreshold=5.0, double approachMeterPerSecond=3.0);

    /*
     * pass a vehicle to the tracker and return whether it is (still) relevant
     */
    bool handleVehicle(const CAMRecord& vehicleRecord);
    /*
     * return whether this tracker currently manages any vehicles
     */
    bool empty() const;
    /*
     * remove all records of vehicles that have not been updated for some time
     */
    void clearTimedOutVehicles();
    /*
     * remove a vehicle from the tracker by id
     */
    void clearVehicle(const VehicleId& vehId);
    /*
     * return list of vehicle records on the approach
     */
    std::vector<VehicleRecord> vehiclesOnApproach(const ApproachId& approach) const;
    /*
     * return the vehicle record for the given id if the vehicle is known
     */
    VehicleRecord getVehicle(const VehicleId vehicleId) const;
    /*
     * return ids of all approaches to this tracker's junction
     */
    std::vector<ApproachId> getApproaches() const;

private:
    /*
     * register a previously unknown vehicle
     */
    void registerVehicle(const CAMRecord& vehicleRecord);
    /*
     * unregister a vehicle that is no longer relevant to this junction
     */
    void unregisterVehicle(const VehicleId& vehId);
    /*
     * update a vehicle that is still relevant to this junction
     */
    void updateVehicle(const CAMRecord& vehicleRecord);
    /*
     * is the given vehicle relevant to this junction?
     */
    bool isRelevant(const CAMRecord& vehicleRecord) const;
    /*
     * is the given vehicle already known to this junction?
     */
    bool isKnown(const VehicleId& vehId) const;
    /*
     * return the arrival time at the stop line for the given vehicle
     */
    simtime_t estimateArrivalTime(const CAMRecord& vehicleRecord) const;

private:
    /*
     * conflict graph object for this junction
     */
    ConflictGraph conflictGraph;
    /*
     * maximum distance of vehicles to the junction to be considered relevant
     */
    double maxDistance;
    /*
     * time out value after which vehicle records are no longer considered valid
     */
    simtime_t timeOut;
    /*
     * average speed at which vehicles move when approaching a junction
     */
    double approachMeterPerSecond;
    /*
     * threshold distance to consider a vehicle as "at the stop line"
     */
    double junctionProximityThreshold;
    /*
     * vehicles currently considered by the tracker on <key> approach
     */
    std::map<CompoundApproach::approachId, VehicleQueue> approachingVehicles;
    /*
     * map of all currently relevant vehicles to their full records
     */
    std::map<VehicleId, VehicleRecord> knownVehicles;
};

} /* namespace Veins */
#endif /* VEHICLETRACKER_H_ */
