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

#pragma once

#include <map>
#include <memory>
#include <list>
#include <queue>

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"
#include "veins/base/utils/FindModule.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/obstacle/VehicleObstacleControl.h"
#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/VehicleSignal.h"
#include "veins/modules/mobility/traci/TraCIRegionOfInterest.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCIPerson.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCISubscriptionManager.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCITrafficLight.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCIVehicle.h"

namespace veins {

class TraCICommandInterface;

/**
 * @brief
 * Creates and moves nodes controlled by a TraCI server.
 *
 * If the server is a SUMO road traffic simulation, you can use the
 * TraCIScenarioManagerLaunchd module and sumo-launchd.py script instead.
 *
 * All nodes created thus must have a TraCIMobility submodule.
 *
 * See the Veins website <a href="http://veins.car2x.org/"> for a tutorial, documentation, and publications </a>.
 *
 * @author Christoph Sommer, David Eckhoff, Falko Dressler, Zheng Yao, Tobias Mayer, Alvaro Torres Cortes, Luca Bedogni
 *
 * @see TraCIMobility
 * @see TraCIScenarioManagerLaunchd
 *
 */
class VEINS_API TraCIScenarioManager : public cSimpleModule {
public:
    static const simsignal_t traciInitializedSignal;
    static const simsignal_t traciModuleAddedSignal;
    static const simsignal_t traciModuleRemovedSignal;
    static const simsignal_t traciTimestepBeginSignal;
    static const simsignal_t traciTimestepEndSignal;

    TraCIScenarioManager();
    ~TraCIScenarioManager() override;
    int numInitStages() const override
    {
        return std::max(cSimpleModule::numInitStages(), 2);
    }
    void initialize(int stage) override;
    void finish() override;
    void handleMessage(cMessage* msg) override;
    virtual void handleSelfMsg(cMessage* msg);

    bool isConnected() const
    {
        return static_cast<bool>(connection);
    }

    TraCICommandInterface* getCommandInterface() const
    {
        return commandIfc.get();
    }

    bool getAutoShutdownTriggered()
    {
        return autoShutdownTriggered;
    }

    const std::map<std::string, cModule*>& getManagedHosts()
    {
        return hosts;
    }

protected:
    simtime_t connectAt; /**< when to connect to TraCI server (must be the initial timestep of the server) */
    simtime_t firstStepAt; /**< when to start synchronizing with the TraCI server (-1: immediately after connecting) */
    simtime_t updateInterval; /**< time interval of hosts' position updates */
    // maps from vehicle type to moduleType, moduleName, and moduleDisplayString
    typedef std::map<std::string, std::string> TypeMapping;
    TypeMapping moduleType; /**< module type to be used in the simulation for each managed vehicle */
    TypeMapping moduleName; /**< module name to be used in the simulation for each managed vehicle */
    TypeMapping moduleDisplayString; /**< module displayString to be used in the simulation for each managed vehicle */
    std::string host;
    int port;

    std::string trafficLightModuleType; /**< module type to be used in the simulation for each managed traffic light */
    std::string trafficLightModuleName; /**< module name to be used in the simulation for each managed traffic light */
    std::string trafficLightModuleDisplayString; /**< module displayString to be used in the simulation for each managed vehicle */
    std::vector<std::string> trafficLightModuleIds; /**< list of traffic light module ids that is subscribed to (whitelist) */

    bool autoShutdown; /**< Shutdown module as soon as no more vehicles are in the simulation */
    double penetrationRate;
    bool ignoreGuiCommands; /**< whether to ignore all TraCI commands that only make sense when the server has a graphical user interface */
    TraCIRegionOfInterest roi; /**< Can return whether a given position lies within the simulation's region of interest. Modules are destroyed and re-created as managed vehicles leave and re-enter the ROI */
    double areaSum;

    AnnotationManager* annotations;
    std::shared_ptr<TraCIConnection> connection;
    std::shared_ptr<TraCICommandInterface> commandIfc;

    size_t nextNodeVectorIndex; /**< next OMNeT++ module vector index to use */
    std::map<std::string, cModule*> hosts; /**< vector of all hosts managed by us */
    std::set<std::string> unEquippedHosts;
    std::map<std::string, cModule*> trafficLights; /**< vector of all traffic lights managed by us */
    bool autoShutdownTriggered;
    cMessage* connectAndStartTrigger; /**< self-message scheduled for when to connect to TraCI server and start running */
    cMessage* executeOneTimestepTrigger; /**< self-message scheduled for when to next call executeOneTimestep */

    BaseWorldUtility* world;
    std::map<const TraCIMobility*, const VehicleObstacle*> vehicleObstacles;
    VehicleObstacleControl* vehicleObstacleControl;

    /**
     * Stores the subscription manager handling everything related to subscriptions.
     */
    TraCISubscriptionManagement::TraCISubscriptionManager subscriptionManager;

    void executeOneTimestep(); /**< read and execute all commands for the next timestep */

    virtual void init_traci();

    virtual void preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, Heading heading, VehicleSignalSet signals);
    virtual void updateModulePosition(cModule* mod, const Coord& p, const std::string& edge, double speed, Heading heading, VehicleSignalSet signals);
    void addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, Heading heading = Heading::nan, VehicleSignalSet signals = {VehicleSignal::undefined}, double length = 0, double height = 0, double width = 0);
    cModule* getManagedModule(std::string nodeId); /**< returns a pointer to the managed module named moduleName, or 0 if no module can be found */
    void deleteManagedModule(std::string nodeId);

    bool isModuleUnequipped(std::string nodeId); /**< returns true if this vehicle is Unequipped */

    /**
     * parses the vector of module types in ini file
     *
     * in case of inconsistencies the function kills the simulation
     */
    void parseModuleTypes();

    /**
     * transforms a list of mappings of an omnetpp.ini parameter in a list
     */
    TypeMapping parseMappings(std::string parameter, std::string parameterName, bool allowEmpty = false);

    /**
     * Helper method that takes a list of updated vehicles and updates/adds
     * modules according to the vehicle data.
     *
     * @param updatedVehicles the list of vehicles.
     */
    void processUpdatedVehicles(std::list<TraCISubscriptionManagement::TraCIVehicle>& updatedVehicles);

    /**
     * Helper method that takes a list of updated person and updates/adds
     * modules according to the person data.
     *
     * @param updatedPersons the list of persons.
     */
    void processUpdatedPersons(std::list<TraCISubscriptionManagement::TraCIPerson>& updatedPersons);

    /**
     * Helper method that takes a list of updated traffic lights and updates
     * the state of the traffic light modules.
     */
    void processUpdatedTrafficLights(std::list<TraCISubscriptionManagement::TraCITrafficLight>& updatedTrafficLights);

    /**
     * Helper method that takes the response buffer of a simstep command and
     * calls the subscription manager and updates modules according to results.
     */
    void processSubscriptions(TraCIBuffer& buffer);

};

class VEINS_API TraCIScenarioManagerAccess {
public:
    TraCIScenarioManager* get()
    {
        return FindModule<TraCIScenarioManager*>::findGlobalModule();
    };
};

} // namespace Veins
