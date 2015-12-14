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

#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>


#define MYDEBUG EV

#include "veins/modules/mobility/traci/TraCIScenarioManagerBase.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCINodeManager.h"
#include "veins/modules/mobility/traci/TraCITime.h"
#include "veins/modules/mobility/traci/TraCIVehicleInserter.h"

using Veins::TraCIScenarioManagerBase;
using Veins::TraCIBuffer;
using Veins::TraCICoord;


TraCIScenarioManagerBase::TraCIScenarioManagerBase() :
		connection(0),
		activeVehicleCount(0),
		parkingVehicleCount(0),
		drivingVehicleCount(0),
		connectAndStartTrigger(0),
		executeOneTimestepTrigger(0),
		vehicleInserter(0)
{
}

TraCIScenarioManagerBase::~TraCIScenarioManagerBase() {
	cancelAndDelete(connectAndStartTrigger);
	cancelAndDelete(executeOneTimestepTrigger);
	delete commandIfc;
	delete connection;
	delete vehicleInserter;
}

void TraCIScenarioManagerBase::initialize(int stage) {
	cSimpleModule::initialize(stage);

	if (stage == INIT_LAST) {
		if (!nodes) {
			error("No TraCINodeManager has been configured during INIT_BASE");
		}
	}
	if (stage != INIT_BASE) {
		return;
	}

	connectAt = par("connectAt");
	firstStepAt = par("firstStepAt");
	updateInterval = par("updateInterval");
	if (firstStepAt == -1) firstStepAt = connectAt + updateInterval;
	penetration.setTargetPenetration(par("penetrationRate").doubleValue());
	host = par("host").stdstringValue();
	port = par("port");
	autoShutdown = par("autoShutdown");

	roi.clear();
	roi.addRoads(par("roiRoads"));
	roi.addRectangles(par("roiRects"));

	subscribedVehicles.clear();
	activeVehicleCount = 0;
	parkingVehicleCount = 0;
	drivingVehicleCount = 0;
	autoShutdownTriggered = false;

	ASSERT(firstStepAt > connectAt);
	connectAndStartTrigger = new cMessage("connect");
	scheduleAt(connectAt, connectAndStartTrigger);
	executeOneTimestepTrigger = new cMessage("step");
	scheduleAt(firstStepAt, executeOneTimestepTrigger);

	MYDEBUG << "initialized TraCIScenarioManagerBase" << endl;
}

void TraCIScenarioManagerBase::init_traci() {
	checkApiCompatibility();
	queryNetworkBoundary();
	subscribeSimulationVariables();
	subscribeVehicleList();

	vehicleInserter = new TraCIVehicleInserter(commandIfc, getRNG(par("vehicleRngIndex")));
	vehicleInserter->useRouteDistributions(par("useRouteDistributions"));
	vehicleInserter->setTargetVehicles(par("numVehicles"));

	for (std::list<TraCIListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		TraCIListener* listener = *it;
		listener->init();
	}
}

void TraCIScenarioManagerBase::finish() {
	if (connection) {
		TraCIBuffer buf = connection->query(CMD_CLOSE, TraCIBuffer());
	}

	if (nodes) {
		nodes->finish();
	}

	for (std::list<TraCIListener*>::reverse_iterator it = listeners.rbegin(); it != listeners.rend(); ++it) {
		TraCIListener* listener = *it;
		listener->close();
	}
}

void TraCIScenarioManagerBase::handleMessage(cMessage *msg) {
	if (msg->isSelfMessage()) {
		handleSelfMsg(msg);
		return;
	}
	error("TraCIScenarioManagerBase doesn't handle messages from other modules");
}

void TraCIScenarioManagerBase::handleSelfMsg(cMessage *msg) {
	if (msg == connectAndStartTrigger) {
		connection = TraCIConnection::connect(host.c_str(), port);
		commandIfc = new TraCICommandInterface(*connection);
		init_traci();
		return;
	}
	if (msg == executeOneTimestepTrigger) {
		executeOneTimestep();
		return;
	}
	error("TraCIScenarioManagerBase received unknown self-message");
}

void TraCIScenarioManagerBase::queryNetworkBoundary() {
	TraCIBuffer buf = connection->query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_NET_BOUNDING_BOX) << std::string("sim0"));
	uint8_t cmdLength_resp; buf >> cmdLength_resp;
	uint8_t commandId_resp; buf >> commandId_resp; ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
	uint8_t variableId_resp; buf >> variableId_resp; ASSERT(variableId_resp == VAR_NET_BOUNDING_BOX);
	std::string simId; buf >> simId;
	uint8_t typeId_resp; buf >> typeId_resp; ASSERT(typeId_resp == TYPE_BOUNDINGBOX);
	double x1; buf >> x1;
	double y1; buf >> y1;
	double x2; buf >> x2;
	double y2; buf >> y2;
	ASSERT(buf.eof());

	TraCICoord netbounds1 = TraCICoord(x1, y1);
	TraCICoord netbounds2 = TraCICoord(x2, y2);
	MYDEBUG << "TraCI reports network boundaries (" << x1 << ", " << y1 << ")-(" << x2 << ", " << y2 << ")" << endl;
	commandIfc->netBoundary().set(netbounds1, netbounds2, par("margin"));
}

// name: host;Car;i=vehicle.gif
void TraCIScenarioManagerBase::addModule(const std::string& nodeId, const TraCINodeManager::NodeData& nodeData) {
	if (nodes->get(nodeId)) error("tried adding duplicate module");
	vehicleInserter->dequeVehicle(nodeId);

	if (!penetration.becomesUnequipped(nodeId, nodes->size())) {
		const std::string& vType = getCommandInterface()->vehicle(nodeId).getTypeId();
		nodes->add(nodeId, nodeData, vType, simTime() + updateInterval);
	}
}

void TraCIScenarioManagerBase::executeOneTimestep() {
	MYDEBUG << "Triggering TraCI server simulation advance to t=" << simTime() << endl;

	const simtime_t now = simTime();
	if (now >= connectAt) {
		if (now > 1.0) {
			vehicleInserter->enqueueVehicles(activeVehicleCount, now);
		}
		TraCIBuffer buf = connection->query(CMD_SIMSTEP2, TraCIBuffer() << TraCITime::from(now));

		uint32_t count; buf >> count;
		MYDEBUG << "Getting " << count << " subscription results" << endl;
		for (uint32_t i = 0; i < count; ++i) {
			processSubcriptionResult(buf);
		}
	}

	if (!autoShutdownTriggered) scheduleAt(now + updateInterval, executeOneTimestepTrigger);

	for (std::list<TraCIListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		TraCIListener* listener = *it;
		listener->step();
	}
}

void TraCIScenarioManagerBase::checkApiCompatibility()
{
	std::pair<uint32_t, std::string> version = getCommandInterface()->getVersion();
	uint32_t apiVersion = version.first;
	std::string serverVersion = version.second;

	if ((apiVersion == 10) || (apiVersion == 11)) {
		MYDEBUG << "TraCI server \"" << serverVersion << "\" reports API version " << apiVersion << endl;
	} else {
		error("TraCI server \"%s\" reports API version %d, which is unsupported. We recommend using SUMO 0.26.0.", serverVersion.c_str(), apiVersion);
	}
}

void TraCIScenarioManagerBase::subscribeSimulationVariables() {
	// subscribe to list of departed and arrived vehicles, as well as simulation time
	static const std::string objectIdIgnored = "";
	static const uint8_t variables[] = {
		VAR_DEPARTED_VEHICLES_IDS,
		VAR_ARRIVED_VEHICLES_IDS,
		VAR_TIME_STEP,
		VAR_TELEPORT_STARTING_VEHICLES_IDS,
		VAR_TELEPORT_ENDING_VEHICLES_IDS,
		VAR_PARKING_STARTING_VEHICLES_IDS,
		VAR_PARKING_ENDING_VEHICLES_IDS
	};
	static const uint8_t variableNumber = sizeof(variables);

	TraCIBuffer request;
	request << TraCITime::min() << TraCITime::max() << objectIdIgnored << variableNumber;
	for (unsigned i = 0; i < sizeof(variables); ++i) {
	    request << variables[i];
	}
	TraCIBuffer response = connection->query(CMD_SUBSCRIBE_SIM_VARIABLE, request);
	processSubcriptionResult(response);
	ASSERT(response.eof());
}

void TraCIScenarioManagerBase::subscribeVehicleList() {
	// subscribe to list of vehicle ids
	static const std::string objectIdIgnored = "";
	static const uint8_t variableNumber = 1;
	static const uint8_t variable = ID_LIST;

	TraCIBuffer request;
	request << TraCITime::min() << TraCITime::max() << objectIdIgnored << variableNumber << variable;
	TraCIBuffer response = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, request);
	processSubcriptionResult(response);
	ASSERT(response.eof());
}

void TraCIScenarioManagerBase::subscribeToVehicleVariables(std::string vehicleId) {
	// subscribe to some attributes of the vehicle
	static const uint8_t variables[] = {
		VAR_POSITION,
		VAR_ROAD_ID,
		VAR_SPEED,
		VAR_ANGLE,
		VAR_SIGNALS
	};
	static const uint8_t variableNumber = sizeof(variables);

	TraCIBuffer request;
	request << TraCITime::min() << TraCITime::max() << vehicleId << variableNumber;
	for (unsigned i = 0; i < variableNumber; ++i) {
		request << variables[i];
	}
	TraCIBuffer response = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, request);
	processSubcriptionResult(response);
	ASSERT(response.eof());
}

void TraCIScenarioManagerBase::unsubscribeFromVehicleVariables(std::string vehicleId) {
	// unsubscribe from all vehicle attributes
	uint8_t variableNumber = 0;

	TraCIBuffer request;
	request << TraCITime::min() << TraCITime::max() << vehicleId << variableNumber;
	TraCIBuffer response = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, request);
	ASSERT(response.eof());
}

void TraCIScenarioManagerBase::processSimSubscription(std::string objectId, TraCIBuffer& buf) {
	uint8_t variableNumber_resp; buf >> variableNumber_resp;
	for (uint8_t j = 0; j < variableNumber_resp; ++j) {
		uint8_t variable1_resp; buf >> variable1_resp;
		uint8_t isokay; buf >> isokay;
		if (isokay != RTYPE_OK) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRING);
			std::string description; buf >> description;
			if (isokay == RTYPE_NOTIMPLEMENTED) error("TraCI server reported subscribing to variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, description.c_str());
			error("TraCI server reported error subscribing to variable 0x%2x (\"%s\").", variable1_resp, description.c_str());
		}

		if (variable1_resp == VAR_DEPARTED_VEHICLES_IDS) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRINGLIST);
			uint32_t count; buf >> count;
			MYDEBUG << "TraCI reports " << count << " departed vehicles." << endl;
			for (uint32_t i = 0; i < count; ++i) {
				std::string idstring; buf >> idstring;
				// adding modules is handled on the fly when entering/leaving the ROI
			}

			activeVehicleCount += count;
			drivingVehicleCount += count;

		} else if (variable1_resp == VAR_ARRIVED_VEHICLES_IDS) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRINGLIST);
			uint32_t count; buf >> count;
			MYDEBUG << "TraCI reports " << count << " arrived vehicles." << endl;
			for (uint32_t i = 0; i < count; ++i) {
				std::string idstring; buf >> idstring;

				if (subscribedVehicles.find(idstring) != subscribedVehicles.end()) {
					subscribedVehicles.erase(idstring);
					unsubscribeFromVehicleVariables(idstring);
				}

				// check if this object has been deleted already (e.g. because it was outside the ROI)
				cModule* mod = nodes->get(idstring);
				if (mod) nodes->remove(idstring);
				penetration.removeIfUnequipped(idstring);
			}

			if ((count > 0) && (count >= activeVehicleCount) && autoShutdown) autoShutdownTriggered = true;
			activeVehicleCount -= count;
			drivingVehicleCount -= count;

		} else if (variable1_resp == VAR_TELEPORT_STARTING_VEHICLES_IDS) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRINGLIST);
			uint32_t count; buf >> count;
			MYDEBUG << "TraCI reports " << count << " vehicles starting to teleport." << endl;
			for (uint32_t i = 0; i < count; ++i) {
				std::string idstring; buf >> idstring;

				// check if this object has been deleted already (e.g. because it was outside the ROI)
				cModule* mod = nodes->get(idstring);
				if (mod) nodes->remove(idstring);
				penetration.removeIfUnequipped(idstring);
			}

			activeVehicleCount -= count;
			drivingVehicleCount -= count;

		} else if (variable1_resp == VAR_TELEPORT_ENDING_VEHICLES_IDS) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRINGLIST);
			uint32_t count; buf >> count;
			MYDEBUG << "TraCI reports " << count << " vehicles ending teleport." << endl;
			for (uint32_t i = 0; i < count; ++i) {
				std::string idstring; buf >> idstring;
				// adding modules is handled on the fly when entering/leaving the ROI
			}

			activeVehicleCount += count;
			drivingVehicleCount += count;

		} else if (variable1_resp == VAR_PARKING_STARTING_VEHICLES_IDS) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRINGLIST);
			uint32_t count; buf >> count;
			MYDEBUG << "TraCI reports " << count << " vehicles starting to park." << endl;
			for (uint32_t i = 0; i < count; ++i) {
				std::string idstring; buf >> idstring;


				cModule* mod = nodes->get(idstring);
				for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
					cModule* submod = SUBMODULE_ITERATOR_TO_MODULE(iter);
					TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
					if (!mm) continue;
					mm->changeParkingState(true);
				}
			}

			parkingVehicleCount += count;
			drivingVehicleCount -= count;

		} else if (variable1_resp == VAR_PARKING_ENDING_VEHICLES_IDS) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRINGLIST);
			uint32_t count; buf >> count;
			MYDEBUG << "TraCI reports " << count << " vehicles ending to park." << endl;
			for (uint32_t i = 0; i < count; ++i) {
				std::string idstring; buf >> idstring;

				cModule* mod = nodes->get(idstring);
				for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
					cModule* submod = SUBMODULE_ITERATOR_TO_MODULE(iter);
					TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
					if (!mm) continue;
					mm->changeParkingState(false);
				}
			}
			parkingVehicleCount -= count;
			drivingVehicleCount += count;

		} else if (variable1_resp == VAR_TIME_STEP) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_INTEGER);
			uint32_t serverTimestep; buf >> serverTimestep;
			MYDEBUG << "TraCI reports current time step as " << serverTimestep << "ms." << endl;
			ASSERT(TraCITime::from(simTime()) == serverTimestep);

		} else {
			error("Received unhandled sim subscription result");
		}
	}
}

void TraCIScenarioManagerBase::processVehicleSubscription(std::string objectId, TraCIBuffer& buf) {
	bool isSubscribed = (subscribedVehicles.find(objectId) != subscribedVehicles.end());
	double px;
	double py;
	std::string edge;
	double speed;
	double angle_traci;
	int signals;
	int numRead = 0;

	uint8_t variableNumber_resp; buf >> variableNumber_resp;
	for (uint8_t j = 0; j < variableNumber_resp; ++j) {
		uint8_t variable1_resp; buf >> variable1_resp;
		uint8_t isokay; buf >> isokay;
		if (isokay != RTYPE_OK) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRING);
			std::string errormsg; buf >> errormsg;
			if (isSubscribed) {
				if (isokay == RTYPE_NOTIMPLEMENTED) error("TraCI server reported subscribing to vehicle variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, errormsg.c_str());
				error("TraCI server reported error subscribing to vehicle variable 0x%2x (\"%s\").", variable1_resp, errormsg.c_str());
			}
		} else if (variable1_resp == ID_LIST) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRINGLIST);
			uint32_t count; buf >> count;
			MYDEBUG << "TraCI reports " << count << " active vehicles." << endl;
			ASSERT(count == activeVehicleCount);
			std::set<std::string> drivingVehicles;
			for (uint32_t i = 0; i < count; ++i) {
				std::string idstring; buf >> idstring;
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

		} else if (variable1_resp == VAR_POSITION) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == POSITION_2D);
			buf >> px;
			buf >> py;
			numRead++;
		} else if (variable1_resp == VAR_ROAD_ID) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_STRING);
			buf >> edge;
			numRead++;
		} else if (variable1_resp == VAR_SPEED) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_DOUBLE);
			buf >> speed;
			numRead++;
		} else if (variable1_resp == VAR_ANGLE) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_DOUBLE);
			buf >> angle_traci;
			numRead++;
		} else if (variable1_resp == VAR_SIGNALS) {
			uint8_t varType; buf >> varType;
			ASSERT(varType == TYPE_INTEGER);
			buf >> signals;
			numRead++;
		} else {
			error("Received unhandled vehicle subscription result");
		}
	}

	// bail out if we didn't want to receive these subscription results
	if (!isSubscribed) return;

	// make sure we got updates for all attributes
	if (numRead != 5) return;

	TraCINodeManager::NodeData data;
	data.position = commandIfc->netBoundary().traci2omnet(TraCICoord(px, py));
	if ((data.position.x < 0) || (data.position.y < 0)) {
		error("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)",
			px, py, data.position.x, data.position.y);
	}

	data.angle = commandIfc->netBoundary().traci2omnetAngle(angle_traci);
	data.speed = speed;
	data.road_id = edge;
	data.signals = static_cast<TraCIVehicleSignal::SignalFlags>(signals);

	cModule* mod = nodes->get(objectId);

	// is it in the ROI?
	bool inRoi = !roi.hasConstraints() ? true :
		roi.onAnyRectangle(TraCICoord(px, py)) || roi.partOfRoads(edge);
	if (!inRoi) {
		if (mod) {
			nodes->remove(objectId);
			MYDEBUG << "Vehicle #" << objectId << " left region of interest" << endl;
		} else if (penetration.removeIfUnequipped(objectId)) {
			MYDEBUG << "Vehicle (unequipped) # " << objectId<< " left region of interest" << endl;
		}
		return;
	}

	if (penetration.isUnequipped(objectId)) {
		return;
	}

	if (!mod) {
		// no such module - need to create
		addModule(objectId, data);
		MYDEBUG << "Added vehicle #" << objectId << endl;
	} else {
		// module existed - update position
		nodes->update(objectId, data);
	}
}

void TraCIScenarioManagerBase::processSubcriptionResult(TraCIBuffer& buf) {
	uint8_t cmdLength_resp; buf >> cmdLength_resp;
	uint32_t cmdLengthExt_resp; buf >> cmdLengthExt_resp;
	uint8_t commandId_resp; buf >> commandId_resp;
	std::string objectId_resp; buf >> objectId_resp;

	if (commandId_resp == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE) processVehicleSubscription(objectId_resp, buf);
	else if (commandId_resp == RESPONSE_SUBSCRIBE_SIM_VARIABLE) processSimSubscription(objectId_resp, buf);
	else {
		error("Received unhandled subscription result");
	}
}

void TraCIScenarioManagerBase::addListener(TraCIListener* listener) {
	listeners.push_back(listener);
}

cModule* TraCIScenarioManagerBase::getModule(const std::string& id) {
	return nodes->get(id);
}
