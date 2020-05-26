//
// Copyright (C) 2006 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
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

#include <stdlib.h>

#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins_libsumo/LibsumoCommandInterface.h"
#include "veins_libsumo/LibsumoConnection.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/ParBuffer.h"

// SUMO includes
#include "utils/geom/PositionVector.h"
#include "libsumo/Simulation.h"
#include "libsumo/Vehicle.h"
#include "libsumo/Polygon.h"
#include "libsumo/Lane.h"
#include "libsumo/LaneArea.h"
#include "libsumo/Junction.h"
#include "libsumo/POI.h"
#include "libsumo/Route.h"
#include "libsumo/Edge.h"
#include "libsumo/TrafficLight.h"
#include "utils/common/MsgHandler.h"
#include "utils/common/MsgRetrievingFunction.h"

#ifdef _WIN32
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#endif /* _WIN32 */

using namespace veins::TraCIConstants;

namespace veins {

const std::map<uint32_t, LibsumoCommandInterface::VersionConfig> LibsumoCommandInterface::versionConfigs = {
    {20, {20, TYPE_DOUBLE, TYPE_POLYGON, VAR_TIME}},
    {19, {19, TYPE_DOUBLE, TYPE_POLYGON, VAR_TIME}},
    {18, {18, TYPE_DOUBLE, TYPE_POLYGON, VAR_TIME}},
    {17, {17, TYPE_INTEGER, TYPE_BOUNDINGBOX, VAR_TIME_STEP}},
    {16, {16, TYPE_INTEGER, TYPE_BOUNDINGBOX, VAR_TIME_STEP}},
    {15, {15, TYPE_INTEGER, TYPE_BOUNDINGBOX, VAR_TIME_STEP}},
};

LibsumoCommandInterface::LibsumoCommandInterface(cComponent* owner, LibsumoConnection& c, bool ignoreGuiCommands)
    : HasLogProxy(owner)
    , connection(c)
    , ignoreGuiCommands(ignoreGuiCommands)
{
}

bool LibsumoCommandInterface::isIgnoringGuiCommands()
{
    return ignoreGuiCommands;
}

std::pair<uint32_t, std::string> LibsumoCommandInterface::getVersion()
{

    uint32_t apiVersion = libsumo::TRACI_VERSION;
    std::string serverVersion = std::string("SUMO ") + VERSION_STRING;

    return std::pair<uint32_t, std::string>(apiVersion, serverVersion);
}

void LibsumoCommandInterface::setApiVersion(uint32_t apiVersion)
{
    try {
        versionConfig = versionConfigs.at(apiVersion);
        TraCIBuffer::setTimeType(versionConfig.timeType);
    }
    catch (std::out_of_range const& exc) {
        throw cRuntimeError(std::string("TraCI server reports unsupported TraCI API version: " + std::to_string(apiVersion) + ". We recommend using Sumo version 1.0.1 or 0.32.0").c_str());
    }
}

std::pair<TraCICoord, TraCICoord> LibsumoCommandInterface::initNetworkBoundaries(int margin)
{
    libsumo::TraCIPositionVector bounds = libsumo::Simulation::getNetBoundary();
    libsumo::TraCIPosition bounds_a = bounds[0];
    libsumo::TraCIPosition bounds_b = bounds[1];

    double x1 = bounds[0].x;
    double y1 = bounds[0].y;
    double x2 = bounds[1].x;
    double y2 = bounds[1].y;

    EV_DEBUG << "TraCI reports network boundaries (" << x1 << ", " << y1 << ")-(" << x2 << ", " << y2 << ")" << endl;
    TraCICoord nb1(x1, y1);
    TraCICoord nb2(x2, y2);
    connection.setNetbounds(nb1, nb2, margin);
    return {nb1, nb2};
}

void LibsumoCommandInterface::Vehicle::setSpeedMode(int32_t bitset)
{
    libsumo::Vehicle::setSpeedMode(nodeId, bitset);
}

void LibsumoCommandInterface::Vehicle::setSpeed(double speed)
{
    libsumo::Vehicle::setSpeed(nodeId, speed);
}

void LibsumoCommandInterface::Vehicle::setMaxSpeed(double speed)
{
    libsumo::Vehicle::setMaxSpeed(nodeId, speed);
}

TraCIColor LibsumoCommandInterface::Vehicle::getColor()
{
    libsumo::TraCIColor c = libsumo::Vehicle::getColor(nodeId);
    return TraCIColor(c.r, c.g, c.b, c.a);
}

void LibsumoCommandInterface::Vehicle::setColor(const TraCIColor& color)
{
    libsumo::TraCIColor c(color.red, color.green, color.blue, color.alpha);
    libsumo::Vehicle::setColor(nodeId, c);
}

void LibsumoCommandInterface::Vehicle::slowDown(double speed, simtime_t time)
{
    libsumo::Vehicle::slowDown(nodeId, speed, time.dbl());
}

void LibsumoCommandInterface::Vehicle::newRoute(std::string roadId)
{
    libsumo::Vehicle::changeTarget(nodeId, roadId);
}

void LibsumoCommandInterface::Vehicle::setParking()
{
    throw cRuntimeError("LibsumoCommandInterface::Vehicle::setParking is non-functional as of SUMO 1.0.0");
}

std::list<std::string> LibsumoCommandInterface::getVehicleTypeIds()
{
    std::vector<std::string> o = libsumo::VehicleType::getIDList();
    std::list<std::string> res(o.begin(), o.end());
    return res;
}

double LibsumoCommandInterface::getVehicleTypeMaxSpeed(std::string typeId)
{
    return libsumo::VehicleType::getMaxSpeed(typeId);
}

void LibsumoCommandInterface::setVehicleTypeMaxSpeed(std::string typeId, double maxSpeed)
{
    return libsumo::VehicleType::setMaxSpeed(typeId, maxSpeed);
}

std::list<std::string> LibsumoCommandInterface::getRouteIds()
{
    std::vector<std::string> r = libsumo::Route::getIDList();
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

std::list<std::string> LibsumoCommandInterface::getRoadIds()
{
    std::vector<std::string> r = libsumo::Edge::getIDList();
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

double LibsumoCommandInterface::Road::getCurrentTravelTime()
{
    return libsumo::Edge::getTraveltime(roadId);
}

double LibsumoCommandInterface::Road::getMeanSpeed()
{
    return libsumo::Edge::getLastStepMeanSpeed(roadId);
}

std::string LibsumoCommandInterface::Vehicle::getRoadId()
{
    return libsumo::Vehicle::getRoadID(nodeId);
}

std::string LibsumoCommandInterface::Vehicle::getLaneId()
{
    return libsumo::Vehicle::getLaneID(nodeId);
}

int32_t LibsumoCommandInterface::Vehicle::getLaneIndex()
{
    return libsumo::Vehicle::getLaneIndex(nodeId);
}

std::string LibsumoCommandInterface::Vehicle::getTypeId()
{
    return libsumo::Vehicle::getTypeID(nodeId);
}

double LibsumoCommandInterface::Vehicle::getMaxSpeed()
{
    return libsumo::Vehicle::getMaxSpeed(nodeId);
}

double LibsumoCommandInterface::Vehicle::getLanePosition()
{
    return libsumo::Vehicle::getLanePosition(nodeId);
}

std::list<std::string> LibsumoCommandInterface::Vehicle::getPlannedRoadIds()
{
    std::vector<std::string> r = libsumo::Vehicle::getRoute(nodeId);
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

std::string LibsumoCommandInterface::Vehicle::getRouteId()
{
    return libsumo::Vehicle::getRouteID(nodeId);
}

std::list<std::string> LibsumoCommandInterface::Route::getRoadIds()
{
    std::vector<std::string> r = libsumo::Route::getEdges(routeId);
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

void LibsumoCommandInterface::Vehicle::changeRoute(std::string roadId, simtime_t travelTime)
{

    if (travelTime >= 0) {
        libsumo::Vehicle::setAdaptedTraveltime(nodeId, roadId, travelTime.dbl());
    }
    else {
        libsumo::Vehicle::setAdaptedTraveltime(nodeId, roadId);
    }
    {
        libsumo::Vehicle::rerouteTraveltime(nodeId);
    }
}

double LibsumoCommandInterface::Vehicle::getLength()
{
    return libsumo::Vehicle::getLength(nodeId);
}

double LibsumoCommandInterface::Vehicle::getWidth()
{
    return libsumo::Vehicle::getWidth(nodeId);
}

double LibsumoCommandInterface::Vehicle::getHeight()
{
    return libsumo::Vehicle::getHeight(nodeId);
}

double LibsumoCommandInterface::Vehicle::getAccel()
{
    return libsumo::Vehicle::getAccel(nodeId);
}

double LibsumoCommandInterface::Vehicle::getDeccel()
{
    return libsumo::Vehicle::getDecel(nodeId);
}

double LibsumoCommandInterface::Vehicle::getCO2Emissions() const
{
    return libsumo::Vehicle::getCO2Emission(nodeId);
}

double LibsumoCommandInterface::Vehicle::getCOEmissions() const
{
    return libsumo::Vehicle::getCOEmission(nodeId);
}

double LibsumoCommandInterface::Vehicle::getHCEmissions() const
{
    return libsumo::Vehicle::getHCEmission(nodeId);
}

double LibsumoCommandInterface::Vehicle::getPMxEmissions() const
{
    return libsumo::Vehicle::getPMxEmission(nodeId);
}

double LibsumoCommandInterface::Vehicle::getNOxEmissions() const
{
    return libsumo::Vehicle::getNOxEmission(nodeId);
}

double LibsumoCommandInterface::Vehicle::getFuelConsumption() const
{
    return libsumo::Vehicle::getFuelConsumption(nodeId);
}

double LibsumoCommandInterface::Vehicle::getNoiseEmission() const
{
    return libsumo::Vehicle::getNoiseEmission(nodeId);
}

double LibsumoCommandInterface::Vehicle::getElectricityConsumption() const
{
    return libsumo::Vehicle::getElectricityConsumption(nodeId);
}

double LibsumoCommandInterface::Vehicle::getWaitingTime() const
{
    return libsumo::Vehicle::getWaitingTime(nodeId);
}

double LibsumoCommandInterface::Vehicle::getAccumulatedWaitingTime() const
{
    const auto apiVersion = traci->versionConfig.version;
    if (apiVersion <= 15) {
        throw cRuntimeError("LibsumoCommandInterface::Vehicle::getAccumulatedWaitingTime requires SUMO 0.31.0 or newer");
    }
    return libsumo::Vehicle::getAccumulatedWaitingTime(nodeId);
}

double LibsumoCommandInterface::getDistance(const Coord& p1, const Coord& p2, bool returnDrivingDistance)
{
    TraCICoord tc1 = connection.omnet2traci(p1);
    TraCICoord tc2 = connection.omnet2traci(p2);

    return libsumo::Simulation::getDistance2D(tc1.x, tc1.y, tc2.x, tc2.y, false, returnDrivingDistance);
}

void LibsumoCommandInterface::Vehicle::stopAt(std::string roadId, double pos, uint8_t laneid, double radius, simtime_t waittime)
{
    libsumo::Vehicle::setStop(nodeId, roadId, pos, laneid, waittime.dbl());
}

std::list<std::string> LibsumoCommandInterface::getTrafficlightIds()
{
    std::vector<std::string> r = libsumo::TrafficLight::getIDList();
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

std::string LibsumoCommandInterface::Trafficlight::getCurrentState() const
{
    return libsumo::TrafficLight::getRedYellowGreenState(trafficLightId);
}

simtime_t LibsumoCommandInterface::Trafficlight::getDefaultCurrentPhaseDuration() const
{
    return libsumo::TrafficLight::getPhaseDuration(trafficLightId);
}

std::list<std::string> LibsumoCommandInterface::Trafficlight::getControlledLanes() const
{
    std::vector<std::string> r = libsumo::TrafficLight::getControlledLanes(trafficLightId);
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

std::list<std::list<TraCITrafficLightLink>> LibsumoCommandInterface::Trafficlight::getControlledLinks() const
{
    std::vector<std::vector<libsumo::TraCILink>> o = libsumo::TrafficLight::getControlledLinks(trafficLightId);

    std::list<std::list<TraCITrafficLightLink>> controlledLinks;
    for (auto i : o) {
        std::list<TraCITrafficLightLink> linksOfSignal;
        for (auto j : i) {
            TraCITrafficLightLink link;
            link.incoming = j.fromLane;
            link.outgoing = j.toLane;
            link.internal = j.viaLane;
            linksOfSignal.push_back(link);
        }
        controlledLinks.push_back(linksOfSignal);
    }

    return controlledLinks;
}

int32_t LibsumoCommandInterface::Trafficlight::getCurrentPhaseIndex() const
{
    return libsumo::TrafficLight::getPhase(trafficLightId);
}

std::string LibsumoCommandInterface::Trafficlight::getCurrentProgramID() const
{
    return libsumo::TrafficLight::getProgram(trafficLightId);
}

TraCITrafficLightProgram LibsumoCommandInterface::Trafficlight::getProgramDefinition() const
{

    std::vector<libsumo::TraCILogic> o = libsumo::TrafficLight::getCompleteRedYellowGreenDefinition(trafficLightId);

    TraCITrafficLightProgram program(trafficLightId);
    for (auto i : o) {

        TraCITrafficLightProgram::Logic logic;
        logic.id = i.programID;
        logic.type = i.type;
        logic.parameter = 0;
        logic.currentPhase = i.currentPhaseIndex;

        for (auto j : i.phases) {
            TraCITrafficLightProgram::Phase phase;
            phase.duration = j->duration;
            phase.state = j->state;
            phase.minDuration = j->minDur;
            phase.maxDuration = j->maxDur;
            phase.next = j->next;
            phase.name = j->name;
            logic.phases.push_back(phase);
        }
        program.addLogic(logic);
    }

    return program;
}

simtime_t LibsumoCommandInterface::Trafficlight::getAssumedNextSwitchTime() const
{
    return libsumo::TrafficLight::getNextSwitch(trafficLightId);
}

void LibsumoCommandInterface::Trafficlight::setState(std::string state)
{
    libsumo::TrafficLight::setRedYellowGreenState(trafficLightId, state);
}

void LibsumoCommandInterface::Trafficlight::setPhaseDuration(simtime_t duration)
{
    libsumo::TrafficLight::setPhaseDuration(trafficLightId, duration.dbl());
}

void LibsumoCommandInterface::Trafficlight::setProgramDefinition(TraCITrafficLightProgram::Logic logic, int32_t logicNr)
{

    libsumo::TraCILogic o;
    o.programID = logic.id;
    o.type = logic.type;
    o.currentPhaseIndex = logic.currentPhase;
    o.phases = std::vector<libsumo::TraCIPhase*>();

    for (auto i : logic.phases) {
        libsumo::TraCIPhase* oo = new libsumo::TraCIPhase();

        oo->duration = i.duration.dbl();
        oo->state = i.state;
        oo->minDur = i.minDuration.dbl();
        oo->maxDur = i.maxDuration.dbl();
        oo->next = i.next;
        oo->name = i.name;

        o.phases.push_back(oo);
    }

    o.subParameter = std::map<std::string, std::string>();

    libsumo::TrafficLight::setCompleteRedYellowGreenDefinition(trafficLightId, o);

    for (auto i : o.phases) {
        delete (i);
    }
}

void LibsumoCommandInterface::Trafficlight::setProgram(std::string program)
{
    libsumo::TrafficLight::setProgram(trafficLightId, program);
}

void LibsumoCommandInterface::Trafficlight::setPhaseIndex(int32_t index)
{
    libsumo::TrafficLight::setPhase(trafficLightId, index);
}

std::list<std::string> LibsumoCommandInterface::getPolygonIds()
{
    std::vector<std::string> r = libsumo::Polygon::getIDList();
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

std::string LibsumoCommandInterface::Polygon::getTypeId()
{
    return libsumo::Polygon::getType(polyId);
}

std::list<Coord> LibsumoCommandInterface::Polygon::getShape()
{
    std::vector<libsumo::TraCIPosition> r = libsumo::Polygon::getShape(polyId);
    std::list<Coord> rr;
    for (auto i : r) {
        TraCICoord c(i.x, i.y);
        Coord cc = connection->traci2omnet(c);
        rr.push_back(cc);
    }
    return rr;
}

void LibsumoCommandInterface::Polygon::setShape(const std::list<Coord>& points)
{
    libsumo::TraCIPositionVector shape;
    for (auto i : points) {
        const TraCICoord& pos = connection->omnet2traci(i);
        libsumo::TraCIPosition p;
        p.x = pos.x;
        p.y = pos.y;
        shape.push_back(p);
    }

    libsumo::Polygon::setShape(polyId, shape);
}

void LibsumoCommandInterface::addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<Coord>& points)
{
    libsumo::TraCIPositionVector shape;
    for (auto i : points) {
        const TraCICoord& pos = connection.omnet2traci(i);
        libsumo::TraCIPosition p;
        p.x = pos.x;
        p.y = pos.y;
        shape.push_back(p);
    }

    libsumo::TraCIColor color_t(color.red, color.green, color.blue, color.alpha);

    libsumo::Polygon::add(polyId, shape, color_t, filled, polyType, layer);
}

void LibsumoCommandInterface::Polygon::remove(int32_t layer)
{
    libsumo::Polygon::remove(polyId, layer);
}

std::list<std::string> LibsumoCommandInterface::getPoiIds()
{
    std::vector<std::string> r = libsumo::POI::getIDList();
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

void LibsumoCommandInterface::addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos_)
{
    TraCICoord pos = connection.omnet2traci(pos_);
    libsumo::TraCIColor color_t(color.red, color.green, color.blue, color.alpha);

    libsumo::POI::add(poiId, pos.x, pos.y, color_t, poiType, layer);
}

Coord LibsumoCommandInterface::Poi::getPosition()
{
    libsumo::TraCIPosition o = libsumo::POI::getPosition(poiId);
    TraCICoord c(o.x, o.y);
    Coord cc = connection->traci2omnet(c);
    return cc;
}

void LibsumoCommandInterface::Poi::remove(int32_t layer)
{
    libsumo::POI::remove(poiId, layer);
}

std::list<std::string> LibsumoCommandInterface::getLaneIds()
{
    std::vector<std::string> r = libsumo::Lane::getIDList();
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

std::list<Coord> LibsumoCommandInterface::Lane::getShape()
{
    std::list<Coord> r;
    libsumo::TraCIPositionVector o = libsumo::Lane::getShape(laneId);
    for (auto i : o) {
        TraCICoord cc(i.x, i.y);
        Coord pos = connection->traci2omnet(cc);
        r.push_back(pos);
    }
    return r;
}

std::string LibsumoCommandInterface::Lane::getRoadId()
{
    return libsumo::Lane::getEdgeID(laneId);
}

double LibsumoCommandInterface::Lane::getLength()
{
    return libsumo::Lane::getLength(laneId);
}

double LibsumoCommandInterface::Lane::getMaxSpeed()
{
    return libsumo::Lane::getMaxSpeed(laneId);
}

double LibsumoCommandInterface::Lane::getMeanSpeed()
{
    return libsumo::Lane::getLastStepMeanSpeed(laneId);
}

std::list<std::string> LibsumoCommandInterface::getLaneAreaDetectorIds()
{
    std::vector<std::string> r = libsumo::LaneArea::getIDList();
    std::list<std::string> rr(r.begin(), r.end());
    return rr;
}

int LibsumoCommandInterface::LaneAreaDetector::getLastStepVehicleNumber()
{
    return libsumo::LaneArea::getLastStepVehicleNumber(laneAreaDetectorId);
}

std::list<std::string> LibsumoCommandInterface::getJunctionIds()
{
    std::vector<std::string> o = libsumo::Junction::getIDList();
    std::list<std::string> res(o.begin(), o.end());
    return res;
}

Coord LibsumoCommandInterface::Junction::getPosition()
{
    libsumo::TraCIPosition o = libsumo::Junction::getPosition(junctionId);
    TraCICoord c(o.x, o.y);
    Coord cc = connection->traci2omnet(c);
    return cc;
}

bool LibsumoCommandInterface::addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st, double emitPosition, double emitSpeed, int8_t emitLane)
{

    std::string emitTime;
    if (emitTime_st >= 0) {
        int emitTime_int = floor(emitTime_st.dbl() * 1000);
        std::stringstream emitTime_ss;
        emitTime_ss << emitTime_int;
        emitTime = emitTime_ss.str();
    }
    else if (emitTime_st == DEPART_TIME_TRIGGERED) {
        emitTime = "triggered";
    }
    else if (emitTime_st == DEPART_TIME_CONTAINER_TRIGGERED) {
        emitTime = "containerTriggered";
    }
    else if (emitTime_st == DEPART_TIME_NOW) {
        emitTime = "now";
    }
    else {
        ASSERT(false);
    }

    std::string emitLane_s;
    if (emitLane >= 0) {
        std::stringstream emitLane_ss;
        emitLane_ss << emitLane;
        emitLane_s = emitLane_ss.str();
    }

    else if (emitLane == DEPART_LANE_RANDOM) {
        emitLane_s = "random";
    }
    else if (emitLane == DEPART_LANE_FREE) {
        emitLane_s = "free";
    }
    else if (emitLane == DEPART_LANE_ALLOWED) {
        emitLane_s = "allowed";
    }
    else if (emitLane == DEPART_LANE_BEST) {
        emitLane_s = "best";
    }
    else if (emitLane == DEPART_LANE_FIRST) {
        emitLane_s = "first";
    }
    else {
        ASSERT(false);
    }

    std::string emitPosition_s;
    if (emitPosition >= 0) {
        std::stringstream emitPosition_ss;
        emitPosition_ss << emitLane;
        emitPosition_s = emitPosition_ss.str();
    }
    else if (emitPosition == DEPART_POSITION_RANDOM) {
        emitPosition_s = "random";
    }
    else if (emitLane == DEPART_POSITION_FREE) {
        emitPosition_s = "free";
    }
    else if (emitPosition == DEPART_POSITION_BASE) {
        emitPosition_s = "base";
    }
    else if (emitPosition == DEPART_POSITION_LAST) {
        emitPosition_s = "last";
    }
    else if (emitPosition == DEPART_POSITION_RANDOM_FREE) {
        emitPosition_s = "stop";
    }
    else {
        ASSERT(false);
    }

    std::string emitSpeed_s;
    if (emitSpeed >= 0) {
        std::stringstream emitSpeed_ss;
        emitSpeed_ss << emitLane;
        emitSpeed_s = emitSpeed_ss.str();
    }
    else if (emitSpeed == DEPART_SPEED_RANDOM) {
        emitSpeed_s = "random";
    }
    else if (emitSpeed == DEPART_SPEED_MAX) {
        emitSpeed_s = "max";
    }
    else {
        ASSERT(false);
    }

    libsumo::Vehicle::add(vehicleId, routeId, vehicleTypeId, emitTime, emitLane_s, emitPosition_s, emitSpeed_s);

    return true;
}

bool LibsumoCommandInterface::Vehicle::changeVehicleRoute(const std::list<std::string>& edges)
{
    if (getRoadId().find(':') != std::string::npos) return false;
    if (edges.front().compare(getRoadId()) != 0) return false;

    std::vector<std::string> edgeIDs(edges.begin(), edges.end());
    libsumo::Vehicle::setRoute(nodeId, edgeIDs);

    return true;
}

void LibsumoCommandInterface::Vehicle::setParameter(const std::string& parameter, int value)
{
    std::stringstream strValue;
    strValue << value;
    setParameter(parameter, strValue.str());
}

void LibsumoCommandInterface::Vehicle::setParameter(const std::string& parameter, double value)
{
    std::stringstream strValue;
    strValue << value;
    setParameter(parameter, strValue.str());
}

void LibsumoCommandInterface::Vehicle::setParameter(const std::string& parameter, const std::string& value)
{
    libsumo::Vehicle::setParameter(nodeId, parameter, value);
}

void LibsumoCommandInterface::Vehicle::getParameter(const std::string& parameter, int& value)
{
    std::string v;
    getParameter(parameter, v);
    ParBuffer buf(v);
    buf >> value;
}
void LibsumoCommandInterface::Vehicle::getParameter(const std::string& parameter, double& value)
{
    std::string v;
    getParameter(parameter, v);
    ParBuffer buf(v);
    buf >> value;
}

void LibsumoCommandInterface::Vehicle::getParameter(const std::string& parameter, std::string& value)
{
    value = libsumo::Vehicle::getParameter(nodeId, parameter);
}

std::pair<double, double> LibsumoCommandInterface::getLonLat(const Coord& coord)
{
    TraCICoord tc = connection.omnet2traci(coord);
    auto o = libsumo::Simulation::convertGeo(tc.x, tc.y);

    return std::make_pair(o.x, o.y);
}

std::tuple<std::string, double, uint8_t> LibsumoCommandInterface::getRoadMapPos(const Coord& coord)
{
    TraCICoord tc = connection.omnet2traci(coord);
    auto o = libsumo::Simulation::convertRoad(tc.x, tc.y);

    return std::make_tuple(o.edgeID, o.pos, o.laneIndex);
}

std::list<std::string> LibsumoCommandInterface::getGuiViewIds()
{
    if (ignoreGuiCommands) {
        EV_DEBUG << "Ignoring TraCI GUI command (as instructed by ignoreGuiCommands)" << std::endl;
        return std::list<std::string>();
    }
    // not implemented
    ASSERT(false);
}

std::string LibsumoCommandInterface::GuiView::getScheme()
{
    if (traci->ignoreGuiCommands) {
        EV_DEBUG << "Ignoring TraCI GUI command (as instructed by ignoreGuiCommands)" << std::endl;
        return std::string();
    }
    // not implemented
    ASSERT(false);
}

void LibsumoCommandInterface::GuiView::setScheme(std::string name)
{
    if (traci->ignoreGuiCommands) {
        EV_DEBUG << "Ignoring TraCI GUI command (as instructed by ignoreGuiCommands)" << std::endl;
        return;
    }
    // not implemented
    ASSERT(false);
}

double LibsumoCommandInterface::GuiView::getZoom()
{
    if (traci->ignoreGuiCommands) {
        EV_DEBUG << "Ignoring TraCI GUI command (as instructed by ignoreGuiCommands)" << std::endl;
        return 0;
    }
    // not implemented
    ASSERT(false);
}
void LibsumoCommandInterface::GuiView::setZoom(double zoom)
{
    if (traci->ignoreGuiCommands) {
        EV_DEBUG << "Ignoring TraCI GUI command (as instructed by ignoreGuiCommands)" << std::endl;
        return;
    }
    // not implemented
    ASSERT(false);
}

void LibsumoCommandInterface::GuiView::setBoundary(Coord p1_, Coord p2_)
{
    if (traci->ignoreGuiCommands) {
        EV_DEBUG << "Ignoring TraCI GUI command (as instructed by ignoreGuiCommands)" << std::endl;
        return;
    }
    // not implemented
    ASSERT(false);
}

void LibsumoCommandInterface::GuiView::takeScreenshot(std::string filename, int32_t width, int32_t height)
{
    if (traci->ignoreGuiCommands) {
        EV_DEBUG << "Ignoring TraCI GUI command (as instructed by ignoreGuiCommands)" << std::endl;
        return;
    }
    // not implemented
    ASSERT(false);
}

void LibsumoCommandInterface::GuiView::trackVehicle(std::string vehicleId)
{
    if (traci->ignoreGuiCommands) {
        EV_DEBUG << "Ignoring TraCI GUI command (as instructed by ignoreGuiCommands)" << std::endl;
        return;
    }
    // not implemented
    ASSERT(false);
}

std::string LibsumoCommandInterface::Vehicle::getVType()
{
    return libsumo::Vehicle::getTypeID(nodeId);
}

} // namespace veins
