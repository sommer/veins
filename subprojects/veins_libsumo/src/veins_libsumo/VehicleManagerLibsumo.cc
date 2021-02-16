//
// Copyright (C) 2006-2020 Christoph Sommer <sommer@cms-labs.org>
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

#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <cstdlib>

#include "veins_libsumo/VehicleManagerLibsumo.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins_libsumo/LibsumoCommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins_libsumo/LibsumoMobility.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"

// SUMO includes
#include "utils/geom/PositionVector.h"
#include "libsumo/Simulation.h"
#include "libsumo/Vehicle.h"
#include "libsumo/TrafficLight.h"
#include "utils/common/MsgHandler.h"
#include "utils/common/MsgRetrievingFunction.h"

// veins_libsumo includes
#include "veins_libsumo/MyMessageRetriever.h"

using namespace veins::TraCIConstants;

using veins::AnnotationManagerAccess;
using veins::TraCIBuffer;
using veins::TraCICoord;
using veins::TraCITrafficLightInterface;
using veins::VehicleManagerLibsumo;

Define_Module(veins::VehicleManagerLibsumo);

#define LIBSUMO_EXCEPTION_WRAP(x)                                      \
    try {                                                              \
        x                                                              \
    }                                                                  \
    catch (libsumo::TraCIException e) {                                \
        std::string what = std::string("SUMO Exception: ") + e.what(); \
        std::throw_with_nested(cRuntimeError(what.c_str()));           \
    }

namespace {

template <typename T>
inline std::string replace(std::string haystack, std::string needle, T newValue)
{
    size_t i = haystack.find(needle, 0);
    if (i == std::string::npos) return haystack;
    std::ostringstream os;
    os << newValue;
    haystack.replace(i, needle.length(), os.str());
    return haystack;
}

} // namespace

const simsignal_t VehicleManagerLibsumo::traciInitializedSignal = registerSignal("org_car2x_veins_modules_mobility_traciInitialized");
const simsignal_t VehicleManagerLibsumo::traciModulePreInitSignal = registerSignal("org_car2x_veins_modules_mobility_traciModulePreInit");
const simsignal_t VehicleManagerLibsumo::traciModuleAddedSignal = registerSignal("org_car2x_veins_modules_mobility_traciModuleAdded");
const simsignal_t VehicleManagerLibsumo::traciModuleRemovedSignal = registerSignal("org_car2x_veins_modules_mobility_traciModuleRemoved");
const simsignal_t VehicleManagerLibsumo::traciTimestepBeginSignal = registerSignal("org_car2x_veins_modules_mobility_traciTimestepBegin");
const simsignal_t VehicleManagerLibsumo::traciTimestepEndSignal = registerSignal("org_car2x_veins_modules_mobility_traciTimestepEnd");

VehicleManagerLibsumo::VehicleManagerLibsumo()
    : connection(nullptr)
    , commandIfc(nullptr)
    , connectAndStartTrigger(nullptr)
    , executeOneTimestepTrigger(nullptr)
    , world(nullptr)
    , myMessageRetriever(nullptr)
{
}

VehicleManagerLibsumo::~VehicleManagerLibsumo()
{
    if (connection) {
        killServer();
    }
    if (connectAndStartTrigger) {
        cancelAndDelete(connectAndStartTrigger);
        connectAndStartTrigger = nullptr;
    }
    if (executeOneTimestepTrigger) {
        cancelAndDelete(executeOneTimestepTrigger);
        executeOneTimestepTrigger = nullptr;
    }
}

namespace {

std::vector<std::string> getMapping(std::string el)
{

    // search for string protection characters '
    char protection = '\'';
    size_t first = el.find(protection);
    size_t second;
    size_t eq;
    std::string type, value;
    std::vector<std::string> mapping;

    if (first == std::string::npos) {
        // there's no string protection, simply split by '='
        cStringTokenizer stk(el.c_str(), "=");
        mapping = stk.asVector();
    }
    else {
        // if there's string protection, we need to find a matching delimiter
        second = el.find(protection, first + 1);
        // ensure that a matching delimiter exists, and that it is at the end
        if (second == std::string::npos || second != el.size() - 1) throw cRuntimeError("invalid syntax for mapping \"%s\"", el.c_str());

        // take the value of the mapping as the text within the quotes
        value = el.substr(first + 1, second - first - 1);

        if (first == 0) {
            // if the string starts with a quote, there's only the value
            mapping.push_back(value);
        }
        else {
            // search for the equal sign
            eq = el.find('=');
            // this must be the character before the quote
            if (eq == std::string::npos || eq != first - 1) {
                throw cRuntimeError("invalid syntax for mapping \"%s\"", el.c_str());
            }
            else {
                type = el.substr(0, eq);
            }
            mapping.push_back(type);
            mapping.push_back(value);
        }
    }
    return mapping;
}

} // namespace

VehicleManagerLibsumo::TypeMapping VehicleManagerLibsumo::parseMappings(std::string parameter, std::string parameterName, bool allowEmpty)
{

    /**
     * possible syntaxes
     *
     * "a"          : assign module type "a" to all nodes (for backward compatibility)
     * "a=b"        : assign module type "b" to vehicle type "a". the presence of any other vehicle type in the simulation will cause the simulation to stop
     * "a=b c=d"    : assign module type "b" to vehicle type "a" and "d" to "c". the presence of any other vehicle type in the simulation will cause the simulation to stop
     * "a=b c=d *=e": everything which is not of vehicle type "a" or "b", assign module type "e"
     * "a=b c=0"    : for vehicle type "c" no module should be instantiated
     * "a=b c=d *=0": everything which is not of vehicle type a or c should not be instantiated
     *
     * For display strings key-value pairs needs to be protected with single quotes, as they use an = sign as the type mappings. For example
     * *.manager.moduleDisplayString = "'i=block/process'"
     * *.manager.moduleDisplayString = "a='i=block/process' b='i=misc/sun'"
     *
     * moduleDisplayString can also be left empty:
     * *.manager.moduleDisplayString = ""
     */

    unsigned int i;
    TypeMapping map;

    // tokenizer to split into mappings ("a=b c=d", -> ["a=b", "c=d"])
    cStringTokenizer typesTz(parameter.c_str(), " ");
    // get all mappings
    std::vector<std::string> typeMappings = typesTz.asVector();
    // and check that there exists at least one
    if (typeMappings.size() == 0) {
        if (!allowEmpty)
            throw cRuntimeError("parameter \"%s\" is empty", parameterName.c_str());
        else
            return map;
    }

    // loop through all mappings
    for (i = 0; i < typeMappings.size(); i++) {

        // tokenizer to find the mapping from vehicle type to module type
        std::string typeMapping = typeMappings[i];

        std::vector<std::string> mapping = getMapping(typeMapping);

        if (mapping.size() == 1) {
            // we are where there is no actual assignment
            // "a": this is good
            // "a b=c": this is not
            if (typeMappings.size() != 1)
                // stop simulation with an error
                throw cRuntimeError("parameter \"%s\" includes multiple mappings, but \"%s\" is not mapped to any vehicle type", parameterName.c_str(), mapping[0].c_str());
            else
                // all vehicle types should be instantiated with this module type
                map["*"] = mapping[0];
        }
        else {

            // check that mapping is valid (a=b and not like a=b=c)
            if (mapping.size() != 2) throw cRuntimeError("invalid syntax for mapping \"%s\" for parameter \"%s\"", typeMapping.c_str(), parameterName.c_str());
            // check that the mapping does not already exist
            if (map.find(mapping[0]) != map.end()) throw cRuntimeError("duplicated mapping for vehicle type \"%s\" for parameter \"%s\"", mapping[0].c_str(), parameterName.c_str());

            // finally save the mapping
            map[mapping[0]] = mapping[1];
        }
    }

    return map;
}

void VehicleManagerLibsumo::parseModuleTypes()
{

    TypeMapping::iterator i;
    std::vector<std::string> typeKeys, nameKeys, displayStringKeys;

    std::string moduleTypes = par("moduleType").stdstringValue();
    std::string moduleNames = par("moduleName").stdstringValue();
    std::string moduleDisplayStrings = par("moduleDisplayString").stdstringValue();

    moduleType = parseMappings(moduleTypes, "moduleType", false);
    moduleName = parseMappings(moduleNames, "moduleName", false);
    moduleDisplayString = parseMappings(moduleDisplayStrings, "moduleDisplayString", true);

    // perform consistency check. for each vehicle type in moduleType there must be a vehicle type
    // in moduleName (and in moduleDisplayString if moduleDisplayString is not empty)

    // get all the keys
    for (i = moduleType.begin(); i != moduleType.end(); i++) typeKeys.push_back(i->first);
    for (i = moduleName.begin(); i != moduleName.end(); i++) nameKeys.push_back(i->first);
    for (i = moduleDisplayString.begin(); i != moduleDisplayString.end(); i++) displayStringKeys.push_back(i->first);

    // sort them (needed for intersection)
    std::sort(typeKeys.begin(), typeKeys.end());
    std::sort(nameKeys.begin(), nameKeys.end());
    std::sort(displayStringKeys.begin(), displayStringKeys.end());

    std::vector<std::string> intersection;

    // perform set intersection
    std::set_intersection(typeKeys.begin(), typeKeys.end(), nameKeys.begin(), nameKeys.end(), std::back_inserter(intersection));
    if (intersection.size() != typeKeys.size() || intersection.size() != nameKeys.size()) throw cRuntimeError("keys of mappings of moduleType and moduleName are not the same");

    if (displayStringKeys.size() == 0) return;

    intersection.clear();
    std::set_intersection(typeKeys.begin(), typeKeys.end(), displayStringKeys.begin(), displayStringKeys.end(), std::back_inserter(intersection));
    if (intersection.size() != displayStringKeys.size()) throw cRuntimeError("keys of mappings of moduleType and moduleName are not the same");
}

void VehicleManagerLibsumo::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage != 1) {
        return;
    }

    trafficLightModuleType = par("trafficLightModuleType").stdstringValue();
    trafficLightModuleName = par("trafficLightModuleName").stdstringValue();
    trafficLightModuleDisplayString = par("trafficLightModuleDisplayString").stdstringValue();
    trafficLightModuleIds.clear();
    std::istringstream filterstream(par("trafficLightFilter").stdstringValue());
    std::copy(std::istream_iterator<std::string>(filterstream), std::istream_iterator<std::string>(), std::back_inserter(trafficLightModuleIds));

    connectAt = par("connectAt");
    firstStepAt = par("firstStepAt");
    updateInterval = par("updateInterval");
    if (firstStepAt == -1) firstStepAt = connectAt + updateInterval;
    parseModuleTypes();
    penetrationRate = par("penetrationRate").doubleValue();
    ignoreGuiCommands = par("ignoreGuiCommands");
    commandLine = par("commandLine").stringValue();
    configFile = par("configFile").stringValue();
    seed = par("seed");

    autoShutdown = par("autoShutdown");

    annotations = AnnotationManagerAccess().getIfExists();

    roi.clear();
    roi.addRoads(par("roiRoads"));
    roi.addRectangles(par("roiRects"));

    areaSum = 0;
    nextNodeVectorIndex = 0;
    hosts.clear();
    subscribedVehicles.clear();
    trafficLights.clear();
    activeVehicleCount = 0;
    parkingVehicleCount = 0;
    drivingVehicleCount = 0;
    autoShutdownTriggered = false;

    world = FindModule<BaseWorldUtility*>::findGlobalModule();

    vehicleObstacleControl = FindModule<VehicleObstacleControl*>::findGlobalModule();

    ASSERT(firstStepAt > connectAt);
    connectAndStartTrigger = new cMessage("connect");
    scheduleAt(connectAt, connectAndStartTrigger);
    executeOneTimestepTrigger = new cMessage("step");
    scheduleAt(firstStepAt, executeOneTimestepTrigger);

    EV_DEBUG << "initialized VehicleManagerLibsumo" << endl;
}

void VehicleManagerLibsumo::startServer()
{
    // autoset seed, if requested
    if (seed == -1) {
        const char* seed_s = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNNUMBER);
        seed = atoi(seed_s);
    }

    // assemble commandLine
    commandLine = replace(commandLine, "$configFile", configFile);
    commandLine = replace(commandLine, "$seed", seed);

    myMessageRetriever = new MyMessageRetriever();
    myMessageRetriever->init();

    EV_DEBUG << "Loading simulation" << std::endl;

    std::vector<std::string> options;
    bool is_inside_quotes;
    std::string s;
    std::stringstream ss(commandLine);
    while (std::getline(ss, s, '\"')) {
        if (is_inside_quotes) {
            if (!s.empty()) options.push_back(s);
        }
        else {
            std::stringstream ss2(s);
            while (std::getline(ss2, s, ' '))
                if (!s.empty()) options.push_back(s);
        }
        is_inside_quotes = !is_inside_quotes;
    }

    libsumo::Simulation::load(options);

    EV_DEBUG << "Simulation loaded" << std::endl;
}

void VehicleManagerLibsumo::killServer()
{
    EV_DEBUG << "Closing" << std::endl;

    libsumo::Simulation::close();

    EV_DEBUG << "Deregistering" << std::endl;

    myMessageRetriever->done();

    EV_DEBUG << "Simulation ending" << std::endl;
}

void VehicleManagerLibsumo::init_traci()
{
    auto* commandInterface = getCommandInterface();
    {
        auto apiVersion = commandInterface->getVersion();
        EV_DEBUG << "TraCI server \"" << apiVersion.second << "\" reports API version " << apiVersion.first << endl;
        commandInterface->setApiVersion(apiVersion.first);
    }

    {
        // query and set road network boundaries
        auto networkBoundaries = commandInterface->initNetworkBoundaries(par("margin"));
        if (world != nullptr && ((connection->traci2omnet(networkBoundaries.second).x > world->getPgs()->x) || (connection->traci2omnet(networkBoundaries.first).y > world->getPgs()->y))) {
            EV_DEBUG << "WARNING: Playground size (" << world->getPgs()->x << ", " << world->getPgs()->y << ") might be too small for vehicle at network bounds (" << connection->traci2omnet(networkBoundaries.second).x << ", " << connection->traci2omnet(networkBoundaries.first).y << ")" << endl;
        }
    }

    {
        // subscribe to list of departed and arrived vehicles, as well as simulation time
        std::string objectId = "";
        uint8_t variable1 = VAR_DEPARTED_VEHICLES_IDS;
        uint8_t variable2 = VAR_ARRIVED_VEHICLES_IDS;
        uint8_t variable3 = commandInterface->getTimeStepCmd();
        uint8_t variable4 = VAR_TELEPORT_STARTING_VEHICLES_IDS;
        uint8_t variable5 = VAR_TELEPORT_ENDING_VEHICLES_IDS;
        uint8_t variable6 = VAR_PARKING_STARTING_VEHICLES_IDS;
        uint8_t variable7 = VAR_PARKING_ENDING_VEHICLES_IDS;
        libsumo::Simulation::subscribe({variable1, variable2, variable3, variable4, variable5, variable6, variable7});
    }

    {
        // subscribe to list of vehicle ids
        std::string objectId = "";
        uint8_t variable1 = ID_LIST;
        libsumo::Vehicle::subscribe(objectId, {variable1});
    }

    if (!trafficLightModuleType.empty() && !trafficLightModuleIds.empty()) {
        // initialize traffic lights
        cModule* parentmod = getParentModule();
        if (!parentmod) {
            throw cRuntimeError("Parent Module not found (for traffic light creation)");
        }
        cModuleType* tlModuleType = cModuleType::get(trafficLightModuleType.c_str());

        // query traffic lights via TraCI
        std::list<std::string> trafficLightIds = commandInterface->getTrafficlightIds();
        size_t nrOfTrafficLights = trafficLightIds.size();
        int cnt = 0;
        for (std::list<std::string>::iterator i = trafficLightIds.begin(); i != trafficLightIds.end(); ++i) {
            std::string tlId = *i;
            if (std::find(trafficLightModuleIds.begin(), trafficLightModuleIds.end(), tlId) == trafficLightModuleIds.end()) {
                continue; // filter only selected elements
            }

            Coord position = commandInterface->junction(tlId).getPosition();

            cModule* module = tlModuleType->create(trafficLightModuleName.c_str(), parentmod, nrOfTrafficLights, cnt);
            module->par("externalId") = tlId;
            module->finalizeParameters();
            module->getDisplayString().parse(trafficLightModuleDisplayString.c_str());
            module->buildInside();
            module->scheduleStart(simTime() + updateInterval);

            cModule* tlIfSubmodule = module->getSubmodule("tlInterface");
            // initialize traffic light interface with current program
            TraCITrafficLightInterface* tlIfModule = dynamic_cast<TraCITrafficLightInterface*>(tlIfSubmodule);
            tlIfModule->preInitialize(tlId, position, updateInterval);

            // initialize mobility for positioning
            cModule* mobiSubmodule = module->getSubmodule("mobility");
            mobiSubmodule->par("x") = position.x;
            mobiSubmodule->par("y") = position.y;
            mobiSubmodule->par("z") = position.z;

            module->callInitialize();
            trafficLights[tlId] = module;
            subscribeToTrafficLightVariables(tlId); // subscribe after module is in trafficLights
            cnt++;
        }
    }

    ObstacleControl* obstacles = ObstacleControlAccess().getIfExists();
    if (obstacles) {
        {
            // get list of polygons
            std::list<std::string> ids = commandInterface->getPolygonIds();
            for (std::list<std::string>::iterator i = ids.begin(); i != ids.end(); ++i) {
                std::string id = *i;
                std::string typeId = commandInterface->polygon(id).getTypeId();
                if (!obstacles->isTypeSupported(typeId)) continue;
                std::list<Coord> coords = commandInterface->polygon(id).getShape();
                std::vector<Coord> shape;
                std::copy(coords.begin(), coords.end(), std::back_inserter(shape));
                obstacles->addFromTypeAndShape(id, typeId, shape);
            }
        }
    }

    {
        processSubscriptionResults();
    }

    traciInitialized = true;
    emit(traciInitializedSignal, true);

    // draw and calculate area of rois
    for (std::list<std::pair<TraCICoord, TraCICoord>>::const_iterator r = roi.getRectangles().begin(), end = roi.getRectangles().end(); r != end; ++r) {
        TraCICoord first = r->first;
        TraCICoord second = r->second;

        std::list<Coord> pol;

        Coord a = connection->traci2omnet(first);
        Coord b = connection->traci2omnet(TraCICoord(first.x, second.y));
        Coord c = connection->traci2omnet(second);
        Coord d = connection->traci2omnet(TraCICoord(second.x, first.y));

        pol.push_back(a);
        pol.push_back(b);
        pol.push_back(c);
        pol.push_back(d);

        // draw polygon for region of interest
        if (annotations) {
            annotations->drawPolygon(pol, "black");
        }

        // calculate region area
        double ab = a.distance(b);
        double ad = a.distance(d);
        double area = ab * ad;
        areaSum += area;
    }
}

void VehicleManagerLibsumo::finish()
{
    while (hosts.begin() != hosts.end()) {
        deleteManagedModule(hosts.begin()->first);
    }

    recordScalar("roiArea", areaSum);
}

void VehicleManagerLibsumo::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
        return;
    }
    throw cRuntimeError("VehicleManagerLibsumo doesn't handle messages from other modules");
}

void VehicleManagerLibsumo::handleSelfMsg(cMessage* msg)
{
    if (msg == connectAndStartTrigger) {
        startServer();
        connection.reset(LibsumoConnection::connect(this));
        commandIfc.reset(new LibsumoCommandInterface(this, *connection, ignoreGuiCommands));
        init_traci();
        return;
    }
    if (msg == executeOneTimestepTrigger) {
        executeOneTimestep();
        return;
    }
    throw cRuntimeError("VehicleManagerLibsumo received unknown self-message");
}

void VehicleManagerLibsumo::preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, Heading heading, VehicleSignalSet signals)
{
    // pre-initialize LibsumoMobility
    auto mobilityModules = getSubmodulesOfType<LibsumoMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->preInitialize(nodeId, position, road_id, speed, heading);
    }
}

void VehicleManagerLibsumo::updateModulePosition(cModule* mod, const Coord& p, const std::string& edge, double speed, Heading heading, VehicleSignalSet signals)
{
    // update position in LibsumoMobility
    auto mobilityModules = getSubmodulesOfType<LibsumoMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->nextPosition(p, edge, speed, heading, signals);
    }
}

// name: host;Car;i=vehicle.gif
void VehicleManagerLibsumo::addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id, double speed, Heading heading, VehicleSignalSet signals, double length, double height, double width)
{

    if (hosts.find(nodeId) != hosts.end()) throw cRuntimeError("tried adding duplicate module");

    double option1 = hosts.size() / (hosts.size() + unEquippedHosts.size() + 1.0);
    double option2 = (hosts.size() + 1) / (hosts.size() + unEquippedHosts.size() + 1.0);

    if (fabs(option1 - penetrationRate) < fabs(option2 - penetrationRate)) {
        unEquippedHosts.insert(nodeId);
        return;
    }

    int32_t nodeVectorIndex = nextNodeVectorIndex++;

    cModule* parentmod = getParentModule();
    if (!parentmod) throw cRuntimeError("Parent Module not found");

    cModuleType* nodeType = cModuleType::get(type.c_str());
    if (!nodeType) throw cRuntimeError("Module Type \"%s\" not found", type.c_str());

    // TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    if (displayString.length() > 0) {
        mod->getDisplayString().parse(displayString.c_str());
    }
    mod->buildInside();
    mod->scheduleStart(simTime() + updateInterval);

    preInitializeModule(mod, nodeId, position, road_id, speed, heading, signals);

    emit(traciModulePreInitSignal, mod);

    mod->callInitialize();
    hosts[nodeId] = mod;

    // post-initialize LibsumoMobility
    auto mobilityModules = getSubmodulesOfType<LibsumoMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->changePosition();
    }

    if (vehicleObstacleControl) {
        std::vector<AntennaPosition> initialAntennaPositions;
        for (auto& caModule : getSubmodulesOfType<ChannelAccess>(mod, true)) {
            initialAntennaPositions.push_back(caModule->getAntennaPosition());
        }
        ASSERT(mobilityModules.size() == 1);
        auto mm = mobilityModules[0];
        double offset = mm->getHostPositionOffset();
        const MobileHostObstacle* vo = vehicleObstacleControl->add(MobileHostObstacle(initialAntennaPositions, mm, length, offset, width, height));
        vehicleObstacles[mm] = vo;
    }

    emit(traciModuleAddedSignal, mod);
}

cModule* VehicleManagerLibsumo::getManagedModule(std::string nodeId)
{
    if (hosts.find(nodeId) == hosts.end()) return nullptr;
    return hosts[nodeId];
}

bool VehicleManagerLibsumo::isModuleUnequipped(std::string nodeId)
{
    if (unEquippedHosts.find(nodeId) == unEquippedHosts.end()) return false;
    return true;
}

void VehicleManagerLibsumo::deleteManagedModule(std::string nodeId)
{
    cModule* mod = getManagedModule(nodeId);
    if (!mod) throw cRuntimeError("no vehicle with Id \"%s\" found", nodeId.c_str());

    emit(traciModuleRemovedSignal, mod);

    auto cas = getSubmodulesOfType<ChannelAccess>(mod, true);
    for (auto ca : cas) {
        cModule* nic = ca->getParentModule();
        auto connectionManager = ChannelAccess::getConnectionManager(nic);
        connectionManager->unregisterNic(nic);
    }
    if (vehicleObstacleControl) {
        for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
            cModule* submod = *iter;
            LibsumoMobility* mm = dynamic_cast<LibsumoMobility*>(submod);
            if (!mm) continue;
            auto vo = vehicleObstacles.find(mm);
            ASSERT(vo != vehicleObstacles.end());
            vehicleObstacleControl->erase(vo->second);
        }
    }

    hosts.erase(nodeId);
    mod->callFinish();
    mod->deleteModule();
}

void VehicleManagerLibsumo::executeOneTimestep()
{

    EV_DEBUG << "Triggering TraCI server simulation advance to t=" << simTime() << endl;

    simtime_t targetTime = simTime();

    emit(traciTimestepBeginSignal, targetTime);

    if (isConnected()) {
        LIBSUMO_EXCEPTION_WRAP(libsumo::Simulation::step(targetTime.dbl()); )
        processSubscriptionResults();
    }

    emit(traciTimestepEndSignal, targetTime);

    if (!autoShutdownTriggered) scheduleAt(simTime() + updateInterval, executeOneTimestepTrigger);
}

void VehicleManagerLibsumo::subscribeToVehicleVariables(std::string vehicleId)
{
    // subscribe to some attributes of the vehicle
    std::string objectId = vehicleId;
    uint8_t variable1 = VAR_POSITION;
    uint8_t variable2 = VAR_ROAD_ID;
    uint8_t variable3 = VAR_SPEED;
    uint8_t variable4 = VAR_ANGLE;
    uint8_t variable5 = VAR_SIGNALS;
    uint8_t variable6 = VAR_LENGTH;
    uint8_t variable7 = VAR_HEIGHT;
    uint8_t variable8 = VAR_WIDTH;
    libsumo::Vehicle::subscribe(objectId, {variable1, variable2, variable3, variable4, variable5, variable6, variable7, variable8});

    processVehicleSubscription(objectId, libsumo::Vehicle::getSubscriptionResults(objectId));
}

void VehicleManagerLibsumo::unsubscribeFromVehicleVariables(std::string vehicleId)
{
    // subscribe to some attributes of the vehicle
    std::string objectId = vehicleId;

    libsumo::Vehicle::subscribe(objectId, {});
}
void VehicleManagerLibsumo::subscribeToTrafficLightVariables(std::string tlId)
{
    // subscribe to some attributes of the traffic light system
    std::string objectId = tlId;
    uint8_t variable1 = TL_CURRENT_PHASE;
    uint8_t variable2 = TL_CURRENT_PROGRAM;
    uint8_t variable3 = TL_NEXT_SWITCH;
    uint8_t variable4 = TL_RED_YELLOW_GREEN_STATE;

    libsumo::TrafficLight::subscribe(objectId, {variable1, variable2, variable3, variable4});
    processTrafficLightSubscription(objectId, libsumo::TrafficLight::getSubscriptionResults(objectId));
}

void VehicleManagerLibsumo::unsubscribeFromTrafficLightVariables(std::string tlId)
{
    // unsubscribe from some attributes of the traffic light system
    // this method is mainly for completeness as traffic lights are not supposed to be removed at runtime

    std::string objectId = tlId;

    libsumo::TrafficLight::subscribe(objectId, {});
}

void VehicleManagerLibsumo::processTrafficLightSubscription(std::string objectId, const libsumo::TraCIResults& results)
{
    ASSERT(trafficLights.count(objectId));
    cModule* tlIfSubmodule = trafficLights[objectId]->getSubmodule("tlInterface");
    TraCITrafficLightInterface* tlIfModule = dynamic_cast<TraCITrafficLightInterface*>(tlIfSubmodule);
    if (!tlIfModule) {
        throw cRuntimeError("Could not find traffic light module %s", objectId.c_str());
    }

    for (auto i : results) {
        uint8_t response_type = i.first;
        libsumo::TraCIResult* r = i.second.get();

        switch (response_type) {
        case TL_CURRENT_PHASE:
            tlIfModule->setCurrentPhaseByNr(check_and_cast<libsumo::TraCIInt*>(r)->value, false);
            break;
        case TL_CURRENT_PROGRAM:
            tlIfModule->setCurrentLogicById(check_and_cast<libsumo::TraCIString*>(r)->value, false);
            break;
        case TL_NEXT_SWITCH:
            tlIfModule->setNextSwitch(check_and_cast<libsumo::TraCIDouble*>(r)->value, false);
            break;
        case TL_RED_YELLOW_GREEN_STATE:
            tlIfModule->setCurrentState(check_and_cast<libsumo::TraCIString*>(r)->value, false);
            break;
        default:
            throw cRuntimeError("Received unhandled traffic light subscription result; type: 0x%02x", response_type);
            break;
        }
    }
}

void VehicleManagerLibsumo::processSimSubscription(std::string objectId, const libsumo::TraCIResults& results)
{
    for (auto i : results) {
        uint8_t variable1_resp = i.first;
        libsumo::TraCIResult* r = i.second.get();

        if (variable1_resp == VAR_DEPARTED_VEHICLES_IDS) {
            libsumo::TraCIStringList* rr = check_and_cast<libsumo::TraCIStringList*>(r);

            uint32_t count = rr->value.size();
            EV_DEBUG << "TraCI reports " << count << " departed vehicles." << endl;
            for (auto i : rr->value) {
                std::string idstring = i;
                // adding modules is handled on the fly when entering/leaving the ROI
            }

            activeVehicleCount += count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == VAR_ARRIVED_VEHICLES_IDS) {
            libsumo::TraCIStringList* rr = check_and_cast<libsumo::TraCIStringList*>(r);

            uint32_t count = rr->value.size();
            EV_DEBUG << "TraCI reports " << count << " arrived vehicles." << endl;
            for (auto i : rr->value) {
                std::string idstring = i;

                if (subscribedVehicles.find(idstring) != subscribedVehicles.end()) {
                    subscribedVehicles.erase(idstring);
                    // no unsubscription via TraCI possible/necessary as of SUMO 1.0.0 (the vehicle has arrived)
                }

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);

                if (unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
                    unEquippedHosts.erase(idstring);
                }
            }

            if ((count > 0) && (count >= activeVehicleCount) && autoShutdown) autoShutdownTriggered = true;
            activeVehicleCount -= count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == VAR_TELEPORT_STARTING_VEHICLES_IDS) {
            libsumo::TraCIStringList* rr = check_and_cast<libsumo::TraCIStringList*>(r);

            uint32_t count = rr->value.size();
            EV_DEBUG << "TraCI reports " << count << " vehicles starting to teleport." << endl;
            for (auto i : rr->value) {
                std::string idstring = i;

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);

                if (unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
                    unEquippedHosts.erase(idstring);
                }
            }

            activeVehicleCount -= count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == VAR_TELEPORT_ENDING_VEHICLES_IDS) {
            libsumo::TraCIStringList* rr = check_and_cast<libsumo::TraCIStringList*>(r);

            uint32_t count = rr->value.size();
            EV_DEBUG << "TraCI reports " << count << " vehicles ending teleport." << endl;
            for (auto i : rr->value) {
                std::string idstring = i;
                // adding modules is handled on the fly when entering/leaving the ROI
            }

            activeVehicleCount += count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == VAR_PARKING_STARTING_VEHICLES_IDS) {
            libsumo::TraCIStringList* rr = check_and_cast<libsumo::TraCIStringList*>(r);

            uint32_t count = rr->value.size();
            EV_DEBUG << "TraCI reports " << count << " vehicles starting to park." << endl;
            for (auto i : rr->value) {
                std::string idstring = i;

                cModule* mod = getManagedModule(idstring);
                auto mobilityModules = getSubmodulesOfType<LibsumoMobility>(mod);
                for (auto mm : mobilityModules) {
                    mm->changeParkingState(true);
                }
            }

            parkingVehicleCount += count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == VAR_PARKING_ENDING_VEHICLES_IDS) {
            libsumo::TraCIStringList* rr = check_and_cast<libsumo::TraCIStringList*>(r);

            uint32_t count = rr->value.size();
            EV_DEBUG << "TraCI reports " << count << " vehicles ending to park." << endl;
            for (auto i : rr->value) {
                std::string idstring = i;

                cModule* mod = getManagedModule(idstring);
                auto mobilityModules = getSubmodulesOfType<LibsumoMobility>(mod);
                for (auto mm : mobilityModules) {
                    mm->changeParkingState(false);
                }
            }
            parkingVehicleCount -= count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == getCommandInterface()->getTimeStepCmd()) {
            libsumo::TraCIDouble* rr = check_and_cast<libsumo::TraCIDouble*>(r);
            simtime_t serverTimestep(rr->value);
            EV_DEBUG << "TraCI reports current time step as " << serverTimestep << "ms." << endl;
            simtime_t omnetTimestep = simTime();
            ASSERT(omnetTimestep == serverTimestep);
        }
        else {
            throw cRuntimeError("Received unhandled sim subscription result");
        }
    }
}

void VehicleManagerLibsumo::processVehicleSubscription(std::string objectId, const libsumo::TraCIResults& results)
{
    bool isSubscribed = (subscribedVehicles.find(objectId) != subscribedVehicles.end());
    double px;
    double py;
    std::string edge;
    double speed;
    double angle_traci;
    int signals;
    double length;
    double height;
    double width;
    int numRead = 0;

    for (auto i : results) {
        uint8_t variable1_resp = i.first;
        libsumo::TraCIResult* r = i.second.get();

        if (variable1_resp == ID_LIST) {
            libsumo::TraCIStringList* rr = check_and_cast<libsumo::TraCIStringList*>(r);

            uint32_t count = rr->value.size();
            EV_DEBUG << "TraCI reports " << count << " active vehicles." << endl;
            // XXX:todo remove ASSERT(count == activeVehicleCount);
            std::set<std::string> drivingVehicles;
            for (auto i : rr->value) {
                std::string idstring = i;
                drivingVehicles.insert(idstring);
            }

            // check for vehicles that need subscribing to
            std::set<std::string> needSubscribe;
            std::set_difference(drivingVehicles.begin(), drivingVehicles.end(), subscribedVehicles.begin(), subscribedVehicles.end(), std::inserter(needSubscribe, needSubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needSubscribe.begin(); i != needSubscribe.end(); ++i) {
                subscribedVehicles.insert(*i);
                subscribeToVehicleVariables(*i);
            }

            // check for vehicles that need unsubscribing from
            std::set<std::string> needUnsubscribe;
            std::set_difference(subscribedVehicles.begin(), subscribedVehicles.end(), drivingVehicles.begin(), drivingVehicles.end(), std::inserter(needUnsubscribe, needUnsubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needUnsubscribe.begin(); i != needUnsubscribe.end(); ++i) {
                subscribedVehicles.erase(*i);
                unsubscribeFromVehicleVariables(*i);
            }
        }
        else if (variable1_resp == VAR_POSITION) {

            px = dynamic_cast<libsumo::TraCIPosition*>(r)->x;
            py = dynamic_cast<libsumo::TraCIPosition*>(r)->y;
            numRead++;
        }
        else if (variable1_resp == VAR_ROAD_ID) {
            edge = dynamic_cast<libsumo::TraCIString*>(r)->value;
            numRead++;
        }
        else if (variable1_resp == VAR_SPEED) {
            speed = dynamic_cast<libsumo::TraCIDouble*>(r)->value;
            numRead++;
        }
        else if (variable1_resp == VAR_ANGLE) {
            angle_traci = dynamic_cast<libsumo::TraCIDouble*>(r)->value;
            numRead++;
        }
        else if (variable1_resp == VAR_SIGNALS) {
            signals = dynamic_cast<libsumo::TraCIInt*>(r)->value;
            numRead++;
        }
        else if (variable1_resp == VAR_LENGTH) {
            length = dynamic_cast<libsumo::TraCIDouble*>(r)->value;
            numRead++;
        }
        else if (variable1_resp == VAR_HEIGHT) {
            height = dynamic_cast<libsumo::TraCIDouble*>(r)->value;
            numRead++;
        }
        else if (variable1_resp == VAR_WIDTH) {
            width = dynamic_cast<libsumo::TraCIDouble*>(r)->value;
            numRead++;
        }
        else {
            throw cRuntimeError("Received unhandled vehicle subscription result");
        }
    }

    EV_DEBUG << "vehicle #" << objectId << " moving, list of results has " << results.size() << " members; read " << numRead << endl;

    // bail out if we didn't want to receive these subscription results
    if (!isSubscribed) return;

    // make sure we got updates for all attributes
    if (numRead != 8) return;

    Coord p = connection->traci2omnet(TraCICoord(px, py));
    if ((p.x < 0) || (p.y < 0)) throw cRuntimeError("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);

    Heading heading = connection->traci2omnetHeading(angle_traci);

    cModule* mod = getManagedModule(objectId);

    // is it in the ROI?
    bool inRoi = !roi.hasConstraints() ? true : (roi.onAnyRectangle(TraCICoord(px, py)) || roi.partOfRoads(edge));
    if (!inRoi) {
        if (mod) {
            deleteManagedModule(objectId);
            EV_DEBUG << "Vehicle #" << objectId << " left region of interest" << endl;
        }
        else if (unEquippedHosts.find(objectId) != unEquippedHosts.end()) {
            unEquippedHosts.erase(objectId);
            EV_DEBUG << "Vehicle (unequipped) # " << objectId << " left region of interest" << endl;
        }
        return;
    }

    if (isModuleUnequipped(objectId)) {
        return;
    }

    if (!mod) {
        // no such module - need to create
        std::string vType = commandIfc->vehicle(objectId).getTypeId();
        std::string mType, mName, mDisplayString;
        TypeMapping::iterator iType, iName, iDisplayString;

        TypeMapping::iterator i;
        iType = moduleType.find(vType);
        if (iType == moduleType.end()) {
            iType = moduleType.find("*");
            if (iType == moduleType.end()) throw cRuntimeError("cannot find a module type for vehicle type \"%s\"", vType.c_str());
        }
        mType = iType->second;
        // search for module name
        iName = moduleName.find(vType);
        if (iName == moduleName.end()) {
            iName = moduleName.find(std::string("*"));
            if (iName == moduleName.end()) throw cRuntimeError("cannot find a module name for vehicle type \"%s\"", vType.c_str());
        }
        mName = iName->second;
        if (moduleDisplayString.size() != 0) {
            iDisplayString = moduleDisplayString.find(vType);
            if (iDisplayString == moduleDisplayString.end()) {
                iDisplayString = moduleDisplayString.find("*");
                if (iDisplayString == moduleDisplayString.end()) throw cRuntimeError("cannot find a module display string for vehicle type \"%s\"", vType.c_str());
            }
            mDisplayString = iDisplayString->second;
        }
        else {
            mDisplayString = "";
        }

        if (mType != "0") {
            addModule(objectId, mType, mName, mDisplayString, p, edge, speed, heading, VehicleSignalSet(signals), length, height, width);
            EV_DEBUG << "Added vehicle #" << objectId << endl;
        }
    }
    else {
        // module existed - update position
        EV_DEBUG << "module " << objectId << " moving to " << p.x << "," << p.y << endl;
        updateModulePosition(mod, p, edge, speed, heading, VehicleSignalSet(signals));
    }
}

void VehicleManagerLibsumo::processSubscriptionResults()
{

    for (auto i : libsumo::Vehicle::getAllSubscriptionResults()) {
        LIBSUMO_EXCEPTION_WRAP(processVehicleSubscription(i.first, i.second); )
    }

    libsumo::TraCIResults results2 = libsumo::Simulation::getSubscriptionResults();
    LIBSUMO_EXCEPTION_WRAP(processSimSubscription("", results2); )

    for (auto i : libsumo::TrafficLight::getAllSubscriptionResults()) {
        LIBSUMO_EXCEPTION_WRAP(processTrafficLightSubscription(i.first, i.second); )
    }
}

// ---

/*
   int main()
   {

    libsumo::TraCIPositionVector bounds = libsumo::Simulation::getNetBoundary();
    libsumo::TraCIPosition bounds_a = bounds[0];
    libsumo::TraCIPosition bounds_b = bounds[1];

    EV_DEBUG << "simulation bounds: " << bounds_a.x << "," << bounds_a.y << " - " << bounds_b.x << "," << bounds_b.y << std::endl;

    for (int step = 0; step < 3; ++step) {

        int t = libsumo::Simulation::getCurrentTime();
        auto tt = libsumo::Simulation::getTime();
        EV_DEBUG << "simulation at t=" << tt << ", int t=" << t << std::endl;

        std::vector<std::string> arrivedVehicles = libsumo::Simulation::getArrivedIDList();
        for (auto veh : arrivedVehicles) {
            managedVehicles.erase(std::remove(managedVehicles.begin(), managedVehicles.end(), veh), managedVehicles.end());
        }

        std::vector<std::string> departedVehicles = libsumo::Simulation::getDepartedIDList();
        managedVehicles.insert(managedVehicles.end(), departedVehicles.begin(), departedVehicles.end());
        for (std::string vehicleID : managedVehicles) {
            libsumo::TraCIPosition p = libsumo::Vehicle::getPosition(vehicleID);
            EV_DEBUG << "vehicle " << vehicleID << " at " << p.x << "," << p.y << std::endl;
        }
    }
   }
 */
