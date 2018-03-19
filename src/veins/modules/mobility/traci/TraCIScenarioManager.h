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

#ifndef VEINS_WORLD_TRACI_TRACISCENARIOMANAGER_H
#define VEINS_WORLD_TRACI_TRACISCENARIOMANAGER_H

#include <map>
#include <list>
#include <queue>

#include <omnetpp.h>

#include "veins/base/utils/Coord.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"
#include "veins/base/utils/FindModule.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCICoord.h"

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
namespace Veins {

class TraCICommandInterface;

class TraCIScenarioManager : public cSimpleModule
{
	public:

		enum VehicleSignal {
			VEH_SIGNAL_UNDEF = -1,
			VEH_SIGNAL_NONE = 0,
			VEH_SIGNAL_BLINKER_RIGHT = 1,
			VEH_SIGNAL_BLINKER_LEFT = 2,
			VEH_SIGNAL_BLINKER_EMERGENCY = 4,
			VEH_SIGNAL_BRAKELIGHT = 8,
			VEH_SIGNAL_FRONTLIGHT = 16,
			VEH_SIGNAL_FOGLIGHT = 32,
			VEH_SIGNAL_HIGHBEAM = 64,
			VEH_SIGNAL_BACKDRIVE = 128,
			VEH_SIGNAL_WIPER = 256,
			VEH_SIGNAL_DOOR_OPEN_LEFT = 512,
			VEH_SIGNAL_DOOR_OPEN_RIGHT = 1024,
			VEH_SIGNAL_EMERGENCY_BLUE = 2048,
			VEH_SIGNAL_EMERGENCY_RED = 4096,
			VEH_SIGNAL_EMERGENCY_YELLOW = 8192
		};

		static const std::string TRACI_INITIALIZED_SIGNAL_NAME;

		TraCIScenarioManager();
		~TraCIScenarioManager();
		virtual int numInitStages() const { return std::max(cSimpleModule::numInitStages(), 2); }
		virtual void initialize(int stage);
		virtual void finish();
		virtual void handleMessage(cMessage *msg);
		virtual void handleSelfMsg(cMessage *msg);

		bool isConnected() const { return (connection); }

		TraCICommandInterface* getCommandInterface() const { return commandIfc; }

		bool getAutoShutdownTriggered() {
			return autoShutdownTriggered;
		}

		const std::map<std::string, cModule*>& getManagedHosts() {
			return hosts;
		}

	protected:
		bool debug; /**< whether to emit debug messages */
		simtime_t connectAt; /**< when to connect to TraCI server (must be the initial timestep of the server) */
		simtime_t firstStepAt; /**< when to start synchronizing with the TraCI server (-1: immediately after connecting) */
		simtime_t updateInterval; /**< time interval of hosts' position updates */
		//maps from vehicle type to moduleType, moduleName, and moduleDisplayString
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

		uint32_t vehicleNameCounter;
		std::vector<std::string> vehicleTypeIds;
		std::map<int, std::queue<std::string> > vehicleInsertQueue;
		std::set<std::string> queuedVehicles;
		std::vector<std::string> routeIds;
		int vehicleRngIndex;
		int numVehicles;

		cRNG* mobRng;

		bool autoShutdown; /**< Shutdown module as soon as no more vehicles are in the simulation */
		double penetrationRate;
		std::list<std::string> roiRoads; /**< which roads (e.g. "hwy1 hwy2") are considered to consitute the region of interest, if not empty */
		std::list<std::pair<TraCICoord, TraCICoord> > roiRects; /**< which rectangles (e.g. "0,0-10,10 20,20-30,30) are considered to consitute the region of interest, if not empty */

		double areaSum;

		AnnotationManager* annotations;
		TraCIConnection* connection;
		TraCICommandInterface* commandIfc;

		size_t nextNodeVectorIndex; /**< next OMNeT++ module vector index to use */
		std::map<std::string, cModule*> hosts; /**< vector of all hosts managed by us */
		std::set<std::string> unEquippedHosts;
		std::set<std::string> subscribedVehicles; /**< all vehicles we have already subscribed to */
		std::map<std::string, cModule*> trafficLights; /**< vector of all traffic lights managed by us */
		uint32_t activeVehicleCount; /**< number of vehicles, be it parking or driving **/
		uint32_t parkingVehicleCount; /**< number of parking vehicles, derived from parking start/end events */
		uint32_t drivingVehicleCount; /**< number of driving, as reported by sumo */
		bool autoShutdownTriggered;
		cMessage* connectAndStartTrigger; /**< self-message scheduled for when to connect to TraCI server and start running */
		cMessage* executeOneTimestepTrigger; /**< self-message scheduled for when to next call executeOneTimestep */

		BaseWorldUtility* world;
		BaseConnectionManager* cc;

		void executeOneTimestep(); /**< read and execute all commands for the next timestep */

		virtual void init_traci();

		virtual void preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, double angle, VehicleSignal signals);
		virtual void updateModulePosition(cModule* mod, const Coord& p, const std::string& edge, double speed, double angle, VehicleSignal signals);
		void addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id = "", double speed = -1, double angle = -1, VehicleSignal signals = VehicleSignal::VEH_SIGNAL_UNDEF);
		cModule* getManagedModule(std::string nodeId); /**< returns a pointer to the managed module named moduleName, or 0 if no module can be found */
		void deleteManagedModule(std::string nodeId);

		bool isModuleUnequipped(std::string nodeId); /**< returns true if this vehicle is Unequipped */

		/**
		 * returns whether a given position lies within the simulation's region of interest.
		 * Modules are destroyed and re-created as managed vehicles leave and re-enter the ROI
		 */
		bool isInRegionOfInterest(const TraCICoord& position, std::string road_id, double speed, double angle);

		/**
		 * adds a new vehicle to the queue which are tried to be inserted at the next SUMO time step;
		 */
		void insertNewVehicle();

		/**
		 * tries to add all vehicles in the vehicle queue to SUMO;
		 */
		void insertVehicles();

		void subscribeToVehicleVariables(std::string vehicleId);
		void unsubscribeFromVehicleVariables(std::string vehicleId);
		void processSimSubscription(std::string objectId, TraCIBuffer& buf);
		void processVehicleSubscription(std::string objectId, TraCIBuffer& buf);
		void processSubcriptionResult(TraCIBuffer& buf);

		void subscribeToTrafficLightVariables(std::string tlId);
		void unsubscribeFromTrafficLightVariables(std::string tlId);
		void processTrafficLightSubscription(std::string objectId, TraCIBuffer& buf);
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

	private:
		const omnetpp::simsignal_t traciInitializedSignal;
};
}

namespace Veins {
class TraCIScenarioManagerAccess
{
	public:
		TraCIScenarioManager* get() {
			return FindModule<TraCIScenarioManager*>::findGlobalModule();
		};
};
}

#endif
