//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#ifndef VEINS_WORLD_TRACI_TRACISCENARIOMANAGERBASE_H
#define VEINS_WORLD_TRACI_TRACISCENARIOMANAGERBASE_H

#include <map>
#include <list>
#include <queue>

#include <omnetpp.h>

#include "veins/base/utils/Coord.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"
#include "veins/base/utils/FindModule.h"
#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCICoord.h"
#include "veins/modules/mobility/traci/TraCIListener.h"
#include "veins/modules/mobility/traci/TraCINodeManager.h"
#include "veins/modules/mobility/traci/TraCIPenetration.h"
#include "veins/modules/mobility/traci/TraCIRegionOfInterest.h"

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
class TraCIVehicleInserter;

class TraCIScenarioManagerBase : public cSimpleModule
{
	public:
		enum InitStages {
			INIT_BASE = 0,
			INIT_LISTENER = 1,
			INIT_LAST = INIT_LISTENER
		};

		TraCIScenarioManagerBase();
		virtual ~TraCIScenarioManagerBase();
		virtual int numInitStages() const { return INIT_LAST + 1; }
		virtual void initialize(int stage);
		virtual void finish();
		virtual void handleMessage(cMessage *msg);
		virtual void handleSelfMsg(cMessage *msg);

		bool isConnected() const { return (connection); }

		TraCICommandInterface* getCommandInterface() const { return commandIfc; }

		bool getAutoShutdownTriggered() {
			return autoShutdownTriggered;
		}

		void addListener(TraCIListener*);
		cModule* getModule(const std::string&);

	protected:
		simtime_t connectAt; /**< when to connect to TraCI server (must be the initial timestep of the server) */
		simtime_t firstStepAt; /**< when to start synchronizing with the TraCI server (-1: immediately after connecting) */
		simtime_t updateInterval; /**< time interval of hosts' position updates */
		std::string host;
		int port;

		bool autoShutdown; /**< Shutdown module as soon as no more vehicles are in the simulation */

		TraCIConnection* connection;
		TraCICommandInterface* commandIfc;

		std::set<std::string> subscribedVehicles; /**< all vehicles we have already subscribed to */
		unsigned activeVehicleCount; /**< number of vehicles, be it parking or driving **/
		unsigned parkingVehicleCount; /**< number of parking vehicles, derived from parking start/end events */
		unsigned drivingVehicleCount; /**< number of driving, as reported by sumo */
		bool autoShutdownTriggered;
		cMessage* connectAndStartTrigger; /**< self-message scheduled for when to connect to TraCI server and start running */
		cMessage* executeOneTimestepTrigger; /**< self-message scheduled for when to next call executeOneTimestep */

		void executeOneTimestep(); /**< read and execute all commands for the next timestep */

		virtual void init_traci();

		void addModule(const std::string& nodeId, const TraCINodeManager::NodeData&);

		/**
		 * query road network boundaries from SUMO
		 */
		virtual void queryNetworkBoundary();

		void checkApiCompatibility();
		void subscribeSimulationVariables();
		void subscribeVehicleList();
		void subscribeToVehicleVariables(std::string vehicleId);
		void unsubscribeFromVehicleVariables(std::string vehicleId);
		void processSimSubscription(std::string objectId, TraCIBuffer& buf);
		void processVehicleSubscription(std::string objectId, TraCIBuffer& buf);
		void processSubcriptionResult(TraCIBuffer& buf);

		void setNodeManager(TraCINodeManager* _nodes) { nodes = _nodes; }
		TraCINodeManager* getNodeManager() const { return nodes; }

	private:
		std::list<TraCIListener*> listeners;
		TraCIRegionOfInterest roi;
		TraCINodeManager* nodes;
		TraCIVehicleInserter* vehicleInserter;
		TraCIPenetration penetration;
};

} // namespace Veins

#endif
