//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
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

#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <list>

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"

using namespace veins::TraCIConstants;

using veins::AnnotationManagerAccess;
using veins::TraCIBuffer;
using veins::TraCICoord;
using veins::TraCIScenarioManager;
using veins::TraCITrafficLightInterface;

Define_Module(veins::TraCIScenarioManager);

const simsignal_t TraCIScenarioManager::traciInitializedSignal = registerSignal("org.car2x.veins.modules.mobility.traciInitialized");
const simsignal_t TraCIScenarioManager::traciModuleAddedSignal = registerSignal("org.car2x.veins.modules.mobility.traciModuleAdded");
const simsignal_t TraCIScenarioManager::traciModuleRemovedSignal = registerSignal("org.car2x.veins.modules.mobility.traciModuleRemoved");
const simsignal_t TraCIScenarioManager::traciTimestepBeginSignal = registerSignal("org.car2x.veins.modules.mobility.traciTimestepBegin");
const simsignal_t TraCIScenarioManager::traciTimestepEndSignal = registerSignal("org.car2x.veins.modules.mobility.traciTimestepEnd");

TraCIScenarioManager::TraCIScenarioManager()
    : connection(nullptr)
    , commandIfc(nullptr)
    , connectAndStartTrigger(nullptr)
    , executeOneTimestepTrigger(nullptr)
    , world(nullptr)
    , subscriptionManager(nullptr)
{
}

TraCIScenarioManager::~TraCIScenarioManager()
{
    if (connection) {
        TraCIBuffer buf = connection->query(CMD_CLOSE, TraCIBuffer());
    }
    cancelAndDelete(connectAndStartTrigger);
    cancelAndDelete(executeOneTimestepTrigger);
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

TraCIScenarioManager::TypeMapping TraCIScenarioManager::parseMappings(std::string parameter, std::string parameterName, bool allowEmpty)
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

void TraCIScenarioManager::parseModuleTypes()
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

void TraCIScenarioManager::initialize(int stage)
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
    host = par("host").stdstringValue();
    port = par("port");
    autoShutdown = par("autoShutdown");

    annotations = AnnotationManagerAccess().getIfExists();

    roi.clear();
    roi.addRoads(par("roiRoads"));
    roi.addRectangles(par("roiRects"));

    areaSum = 0;
    nextNodeVectorIndex = 0;
    hosts.clear();
    trafficLights.clear();
    autoShutdownTriggered = false;

    world = FindModule<BaseWorldUtility*>::findGlobalModule();

    vehicleObstacleControl = FindModule<VehicleObstacleControl*>::findGlobalModule();

    ASSERT(firstStepAt > connectAt);
    connectAndStartTrigger = new cMessage("connect");
    scheduleAt(connectAt, connectAndStartTrigger);
    executeOneTimestepTrigger = new cMessage("step");
    scheduleAt(firstStepAt, executeOneTimestepTrigger);

    EV_DEBUG << "initialized TraCIScenarioManager" << endl;
}

void TraCIScenarioManager::init_traci()
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

    // intialize the subscription manager (performs subscriptions)
    subscriptionManager.reset(new veins::TraCISubscriptionManagement::ExecutiveSubscriptionManager(connection, commandIfc, true));

    if (!trafficLightModuleType.empty() && !trafficLightModuleIds.empty()) {
        // initialize traffic lights
        cModule* parentmod = getParentModule();
        if (!parentmod) {
            error("Parent Module not found (for traffic light creation)");
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
            subscriptionManager->subscribeToTrafficLight(tlId); // subscribe after module is in trafficLights
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

void TraCIScenarioManager::finish()
{
    while (hosts.begin() != hosts.end()) {
        deleteManagedModule(hosts.begin()->first);
    }

    recordScalar("roiArea", areaSum);
}

void TraCIScenarioManager::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
        return;
    }
    error("TraCIScenarioManager doesn't handle messages from other modules");
}

void TraCIScenarioManager::handleSelfMsg(cMessage* msg)
{
    if (msg == connectAndStartTrigger) {
        connection.reset(TraCIConnection::connect(this, host.c_str(), port));
        commandIfc.reset(new TraCICommandInterface(this, *connection, ignoreGuiCommands));
        init_traci();
        return;
    }
    if (msg == executeOneTimestepTrigger) {
        executeOneTimestep();
        return;
    }
    error("TraCIScenarioManager received unknown self-message");
}

void TraCIScenarioManager::preInitializeModule(cModule* mod,
    const std::string& nodeId, const Coord& position,
    const std::string& road_id, double speed, Heading heading,
    VehicleSignalSet signals)
{
    // pre-initialize TraCIMobility
    auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->preInitialize(nodeId, position, road_id, speed, heading);
    }
}

void TraCIScenarioManager::updateModulePosition(cModule* mod, const Coord& p, const std::string& edge, double speed, Heading heading, VehicleSignalSet signals)
{
    // update position in TraCIMobility
    auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->nextPosition(p, edge, speed, heading, signals);
    }
}

// name: host;Car;i=vehicle.gif
void TraCIScenarioManager::addModule(std::string nodeId, std::string type,
    std::string name, std::string displayString, const Coord& position,
    std::string road_id, double speed, Heading heading,
    VehicleSignalSet signals, double length, double height, double width)
{

    if (hosts.find(nodeId) != hosts.end()) error("tried adding duplicate module");

    double option1 = hosts.size() / (hosts.size() + unEquippedHosts.size() + 1.0);
    double option2 = (hosts.size() + 1) / (hosts.size() + unEquippedHosts.size() + 1.0);

    if (fabs(option1 - penetrationRate) < fabs(option2 - penetrationRate)) {
        unEquippedHosts.insert(nodeId);
        return;
    }

    int32_t nodeVectorIndex = nextNodeVectorIndex++;

    cModule* parentmod = getParentModule();
    if (!parentmod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get(type.c_str());
    if (!nodeType) error("Module Type \"%s\" not found", type.c_str());

    // TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    if (displayString.length() > 0) {
        mod->getDisplayString().parse(displayString.c_str());
    }
    mod->buildInside();
    mod->scheduleStart(simTime() + updateInterval);

    preInitializeModule(mod, nodeId, position, road_id, speed, heading, signals);

    mod->callInitialize();
    hosts[nodeId] = mod;

    // post-initialize TraCIMobility
    auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->changePosition();
    }

    if (vehicleObstacleControl) {
        auto caModules = getSubmodulesOfType<ChannelAccess>(mod, true);
        ASSERT(mobilityModules.size() == 1);
        auto mm = mobilityModules[0];
        double offset = mm->getHostPositionOffset();
        const VehicleObstacle* vo = vehicleObstacleControl->add(VehicleObstacle(caModules, mm, length, offset, width, height));
        vehicleObstacles[mm] = vo;
    }

    emit(traciModuleAddedSignal, mod);
}

cModule* TraCIScenarioManager::getManagedModule(std::string nodeId)
{
    if (hosts.find(nodeId) == hosts.end()) return nullptr;
    return hosts[nodeId];
}

bool TraCIScenarioManager::isModuleUnequipped(std::string nodeId)
{
    if (unEquippedHosts.find(nodeId) == unEquippedHosts.end()) return false;
    return true;
}

void TraCIScenarioManager::deleteManagedModule(std::string nodeId)
{
    cModule* mod = getManagedModule(nodeId);
    if (!mod) error("no vehicle with Id \"%s\" found", nodeId.c_str());

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
            TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
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

void TraCIScenarioManager::executeOneTimestep()
{

    EV_DEBUG << "Triggering TraCI server simulation advance to t=" << simTime() << endl;

    simtime_t targetTime = simTime();

    emit(traciTimestepBeginSignal, targetTime);

    if (isConnected()) {
        TraCIBuffer buffer = connection->query(CMD_SIMSTEP2, TraCIBuffer() << targetTime);
        processSubscriptions(buffer);
    }

    emit(traciTimestepEndSignal, targetTime);

    if (!autoShutdownTriggered) scheduleAt(simTime() + updateInterval, executeOneTimestepTrigger);
}

void TraCIScenarioManager::processSubscriptions(TraCIBuffer& buffer)
{
    subscriptionManager->processSubscriptionResult(buffer);

    // vehicles first
    std::vector<TraCISubscriptionManagement::Vehicle> updatedVehicles = subscriptionManager->getUpdatedVehicles();
    EV_DEBUG << "Received " << updatedVehicles.size() << " vehicle updates!" << std::endl;
    processUpdatedVehicles(updatedVehicles);
    std::vector<std::string> disappearedVehicles = subscriptionManager->getDisappearedVehicles();
    for (auto vehicleID : disappearedVehicles) {
        // check if this object has been deleted already (e.g. because it was outside the ROI)
        cModule* mod = getManagedModule(vehicleID);
        if (mod) deleteManagedModule(vehicleID);
        EV_DEBUG << "Unsubscribed to vehicle with id " << vehicleID << std::endl;
    }

    // persons next
    std::vector<TraCISubscriptionManagement::Person> updatedPersons = subscriptionManager->getUpdatedPersons();
    EV_DEBUG << "Received " << updatedPersons.size() << " person updates!" << std::endl;
    processUpdatedPersons(updatedPersons);
    std::vector<std::string> disappearedPersons = subscriptionManager->getDisappearedPersons();
    for (auto personID : disappearedPersons) {
        // check if this object has been deleted already (e.g. because it was outside the ROI)
        cModule* mod = getManagedModule(personID);
        if (mod) deleteManagedModule(personID);
        EV_DEBUG << "Unsubscribed to person with id " << personID << std::endl;
    }

    // simulation next

    // remove all modules that are currently teleporting (they will be added automatically again)
    for (auto id : subscriptionManager->getStartedTeleporting()) {
        // check if this object has been deleted already (e.g. because it was outside the ROI)
        cModule* mod = getManagedModule(id);
        if (mod) deleteManagedModule(id);
        EV_DEBUG << "Person with " << id << " started teleporting." << std::endl;
    }

    // change parking state of all cars that started parking
    for (auto id : subscriptionManager->getStartedParking()) {
        cModule* mod = getManagedModule(id);
        auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
        for (auto mm : mobilityModules) {
            mm->changeParkingState(true);
        }
    }

    // change parking state of all cars that stopped parking
    for (auto id : subscriptionManager->getEndedParking()) {
        cModule* mod = getManagedModule(id);
        auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
        for (auto mm : mobilityModules) {
            mm->changeParkingState(false);
        }
    }

    // last traffic lights
    std::vector<TraCISubscriptionManagement::TrafficLight> trafficLightUpdates = subscriptionManager->getTrafficLightUpdates();
    processUpdatedTrafficLights(trafficLightUpdates);

    // clear the api to not receive the same updates again
    subscriptionManager->clearAPI();
}

void TraCIScenarioManager::processUpdatedVehicles(std::vector<TraCISubscriptionManagement::Vehicle>& updatedVehicles)
{
    for (TraCISubscriptionManagement::Vehicle vehicle : updatedVehicles) {

        // Translate coordinates
        // ------------------------------
        Coord p = connection->traci2omnet(TraCICoord(vehicle.x, vehicle.y));
        if ((p.x < 0) || (p.y < 0)) error("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", vehicle.x, vehicle.y, p.x, p.y);

        // Translate heading
        // ------------------------------
        Heading heading = connection->traci2omnetHeading(vehicle.angle);

        // Find corresponding module
        // ------------------------------
        cModule* mod = getManagedModule(vehicle.id);

        // Check region of interest
        // ------------------------------
        bool inRoi = !roi.hasConstraints() ? true : (roi.onAnyRectangle(TraCICoord(vehicle.x, vehicle.y)) || roi.partOfRoads(vehicle.edgeID));
        if (!inRoi) {
            if (mod) {
                deleteManagedModule(vehicle.id);
                EV_DEBUG << "Vehicle #" << vehicle.id << " left region of interest" << endl;
            }
            else if (unEquippedHosts.find(vehicle.id) != unEquippedHosts.end()) {
                unEquippedHosts.erase(vehicle.id);
                EV_DEBUG << "Vehicle (unequipped) # " << vehicle.id << " left region of interest" << endl;
            }
            return;
        }

        // Check if unequipped
        // ------------------------------
        if (isModuleUnequipped(vehicle.id)) {
            return;
        }

        // Update or create module
        // ------------------------------
        if (!mod) {
            // no such module - need to create
            std::string mType, mName, mDisplayString;
            TypeMapping::iterator iType, iName, iDisplayString;

            TypeMapping::iterator i;
            iType = moduleType.find(vehicle.typeID);
            if (iType == moduleType.end()) {
                iType = moduleType.find("*");
                if (iType == moduleType.end()) throw cRuntimeError("cannot find a module type for vehicle type \"%s\"", vehicle.typeID.c_str());
            }
            mType = iType->second;
            // search for module name
            iName = moduleName.find(vehicle.typeID);
            if (iName == moduleName.end()) {
                iName = moduleName.find(std::string("*"));
                if (iName == moduleName.end()) throw cRuntimeError("cannot find a module name for vehicle type \"%s\"", vehicle.typeID.c_str());
            }
            mName = iName->second;
            if (moduleDisplayString.size() != 0) {
                iDisplayString = moduleDisplayString.find(vehicle.typeID);
                if (iDisplayString == moduleDisplayString.end()) {
                    iDisplayString = moduleDisplayString.find("*");
                    if (iDisplayString == moduleDisplayString.end()) throw cRuntimeError("cannot find a module display string for vehicle type \"%s\"", vehicle.typeID.c_str());
                }
                mDisplayString = iDisplayString->second;
            }
            else {
                mDisplayString = "";
            }

            if (mType != "0") {
                addModule(vehicle.id, mType, mName, mDisplayString, p, vehicle.edgeID, vehicle.speed, heading, VehicleSignalSet(vehicle.signals), vehicle.length, vehicle.height, vehicle.width);
                EV_DEBUG << "Added vehicle #" << vehicle.id << endl;
            }
        }
        else {
            // module existed - update position
            EV_DEBUG << "module " << vehicle.id << " moving to " << p.x << "," << p.y << endl;
            updateModulePosition(mod, p, vehicle.edgeID, vehicle.speed, heading, VehicleSignalSet(vehicle.signals));
        }
    }
}

void TraCIScenarioManager::processUpdatedPersons(std::vector<TraCISubscriptionManagement::Person>& updatedPersons)
{
    for (TraCISubscriptionManagement::Person person : updatedPersons) {

        // Translate coordinates
        // ------------------------------

        Coord p = connection->traci2omnet(TraCICoord(person.x, person.y));
        if ((p.x < 0) || (p.y < 0)) error("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", person.x, person.y, p.x, p.y);

        // Translate heading
        // ------------------------------
        Heading heading = connection->traci2omnetHeading(person.angle);

        // Find corresponding module
        // ------------------------------
        cModule* mod = getManagedModule(person.id);

        // Check region of interest
        // ------------------------------
        bool inRoi = !roi.hasConstraints() ? true : (roi.onAnyRectangle(TraCICoord(person.x, person.y)) || roi.partOfRoads(person.edgeID));
        if (!inRoi) {
            if (mod) {
                deleteManagedModule(person.id);
                EV_DEBUG << "person #" << person.id << " left region of interest" << endl;
            }
            else if (unEquippedHosts.find(person.id) != unEquippedHosts.end()) {
                unEquippedHosts.erase(person.id);
                EV_DEBUG << "person (unequipped) # " << person.id << " left region of interest" << endl;
            }
            return;
        }

        // Check if unequipped
        // ------------------------------
        if (isModuleUnequipped(person.id)) {
            return;
        }

        // Update or create module
        // ------------------------------
        if (!mod) {
            // no such module - need to create
            std::string mType, mName, mDisplayString;
            TypeMapping::iterator iType, iName, iDisplayString;

            TypeMapping::iterator i;
            iType = moduleType.find(person.typeID);
            if (iType == moduleType.end()) {
                iType = moduleType.find("*");
                if (iType == moduleType.end()) throw cRuntimeError("cannot find a module type for person type \"%s\"", person.typeID.c_str());
            }
            mType = iType->second;
            // search for module name
            iName = moduleName.find(person.typeID);
            if (iName == moduleName.end()) {
                iName = moduleName.find(std::string("*"));
                if (iName == moduleName.end()) throw cRuntimeError("cannot find a module name for person type \"%s\"", person.typeID.c_str());
            }
            mName = iName->second;
            if (moduleDisplayString.size() != 0) {
                iDisplayString = moduleDisplayString.find(person.typeID);
                if (iDisplayString == moduleDisplayString.end()) {
                    iDisplayString = moduleDisplayString.find("*");
                    if (iDisplayString == moduleDisplayString.end()) throw cRuntimeError("cannot find a module display string for person type \"%s\"", person.typeID.c_str());
                }
                mDisplayString = iDisplayString->second;
            }
            else {
                mDisplayString = "";
            }

            if (mType != "0") {
                addModule(person.id, mType, mName, mDisplayString, p, person.edgeID, person.speed, heading);
                EV_DEBUG << "Added person #" << person.id << endl;
            }
        }
        else {
            // module existed - update position
            EV_DEBUG << "module " << person.id << " moving to " << p.x << "," << p.y << endl;
            updateModulePosition(mod, p, person.edgeID, person.speed, heading, VehicleSignalSet(VehicleSignal::undefined));
        }
    }
}

void TraCIScenarioManager::processUpdatedTrafficLights(std::vector<TraCISubscriptionManagement::TrafficLight>& updatedTrafficLights)
{
    for (TraCISubscriptionManagement::TrafficLight trafficLight : updatedTrafficLights) {

        // first get the module
        cModule* tlIfSubmodule = trafficLights[trafficLight.id]->getSubmodule("tlInterface");
        TraCITrafficLightInterface* tlIfModule = dynamic_cast<TraCITrafficLightInterface*>(tlIfSubmodule);
        if (!tlIfModule) {
            error("Could not find traffic light module %s", trafficLight.id.c_str());
        }
        tlIfModule->setCurrentPhaseByNr(trafficLight.currentPhase, false);
        tlIfModule->setCurrentLogicById(trafficLight.currentProgram, false);
        tlIfModule->setNextSwitch(trafficLight.nextSwitch, false);
        tlIfModule->setCurrentState(trafficLight.redYellowGreenState, false);
    }
}
