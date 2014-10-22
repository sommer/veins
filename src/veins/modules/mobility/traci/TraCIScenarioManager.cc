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

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/mobility/traci/TraCIScenarioManagerInet.h"

using Veins::TraCIScenarioManager;
using Veins::TraCIBuffer;
using Veins::TraCICoord;

Define_Module(Veins::TraCIScenarioManager);

TraCIScenarioManager::TraCIScenarioManager() :
		myAddVehicleTimer(0),
		mobRng(0),
		connection(0),
		connectAndStartTrigger(0),
		executeOneTimestepTrigger(0),
		world(0),
		cc(0)
{
}

TraCIScenarioManager::~TraCIScenarioManager() {
	cancelAndDelete(connectAndStartTrigger);
	cancelAndDelete(executeOneTimestepTrigger);
	cancelAndDelete(myAddVehicleTimer);
	delete commandIfc;
	delete connection;
}

void TraCIScenarioManager::initialize(int stage) {
	cSimpleModule::initialize(stage);
	if (stage != 1) {
		return;
	}


	debug = par("debug");
	connectAt = par("connectAt");
	firstStepAt = par("firstStepAt");
	updateInterval = par("updateInterval");
	if (firstStepAt == -1) firstStepAt = connectAt + updateInterval;
	moduleType = par("moduleType").stdstringValue();
	moduleName = par("moduleName").stdstringValue();
	moduleDisplayString = par("moduleDisplayString").stdstringValue();
	penetrationRate = par("penetrationRate").doubleValue();
	host = par("host").stdstringValue();
	port = par("port");
	autoShutdown = par("autoShutdown");
	std::string roiRoads_s = par("roiRoads");
	std::string roiRects_s = par("roiRects");

	vehicleNameCounter = 0;
	vehicleRngIndex = par("vehicleRngIndex");
	numVehicles = par("numVehicles").longValue();
	mobRng = getRNG(vehicleRngIndex);

	myAddVehicleTimer = new cMessage("myAddVehicleTimer");

	// parse roiRoads
	roiRoads.clear();
	std::istringstream roiRoads_i(roiRoads_s);
	std::string road;
	while (std::getline(roiRoads_i, road, ' ')) {
		roiRoads.push_back(road);
	}

	// parse roiRects
	roiRects.clear();
	std::istringstream roiRects_i(roiRects_s);
	std::string rect;
	while (std::getline(roiRects_i, rect, ' ')) {
		std::istringstream rect_i(rect);
		double x1; rect_i >> x1; ASSERT(rect_i);
		char c1; rect_i >> c1; ASSERT(rect_i);
		double y1; rect_i >> y1; ASSERT(rect_i);
		char c2; rect_i >> c2; ASSERT(rect_i);
		double x2; rect_i >> x2; ASSERT(rect_i);
		char c3; rect_i >> c3; ASSERT(rect_i);
		double y2; rect_i >> y2; ASSERT(rect_i);
		roiRects.push_back(std::pair<TraCICoord, TraCICoord>(TraCICoord(x1, y1), TraCICoord(x2, y2)));
	}

	nextNodeVectorIndex = 0;
	hosts.clear();
	subscribedVehicles.clear();
	activeVehicleCount = 0;
	parkingVehicleCount = 0;
	drivingVehicleCount = 0;
	autoShutdownTriggered = false;

	world = FindModule<BaseWorldUtility*>::findGlobalModule();
	if (world == NULL) error("Could not find BaseWorldUtility module");

	cc = FindModule<BaseConnectionManager*>::findGlobalModule();
	if (cc == NULL) error("Could not find BaseConnectionManager module");

	ASSERT(firstStepAt > connectAt);
	connectAndStartTrigger = new cMessage("connect");
	scheduleAt(connectAt, connectAndStartTrigger);
	executeOneTimestepTrigger = new cMessage("step");
	scheduleAt(firstStepAt, executeOneTimestepTrigger);

	MYDEBUG << "initialized TraCIScenarioManager" << endl;
}

void TraCIScenarioManager::init_traci() {
	{
		std::pair<uint32_t, std::string> version = getCommandInterface()->getVersion();
		uint32_t apiVersion = version.first;
		std::string serverVersion = version.second;

		if (apiVersion == 8) {
			MYDEBUG << "TraCI server \"" << serverVersion << "\" reports API version " << apiVersion << endl;
		}
		else {
			error("TraCI server \"%s\" reports API version %d, which is unsupported. We recommend using SUMO 0.21.0.", serverVersion.c_str(), apiVersion);
		}

	}

	{
		// query road network boundaries
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
		connection->setNetbounds(netbounds1, netbounds2, par("margin"));
		if ((connection->traci2omnet(netbounds2).x > world->getPgs()->x) || (connection->traci2omnet(netbounds1).y > world->getPgs()->y)) MYDEBUG << "WARNING: Playground size (" << world->getPgs()->x << ", " << world->getPgs()->y << ") might be too small for vehicle at network bounds (" << connection->traci2omnet(netbounds2).x << ", " << connection->traci2omnet(netbounds1).y << ")" << endl;
	}

	{
		// subscribe to list of departed and arrived vehicles, as well as simulation time
		uint32_t beginTime = 0;
		uint32_t endTime = 0x7FFFFFFF;
		std::string objectId = "";
		uint8_t variableNumber = 7;
		uint8_t variable1 = VAR_DEPARTED_VEHICLES_IDS;
		uint8_t variable2 = VAR_ARRIVED_VEHICLES_IDS;
		uint8_t variable3 = VAR_TIME_STEP;
		uint8_t variable4 = VAR_TELEPORT_STARTING_VEHICLES_IDS;
		uint8_t variable5 = VAR_TELEPORT_ENDING_VEHICLES_IDS;
		uint8_t variable6 = VAR_PARKING_STARTING_VEHICLES_IDS;
		uint8_t variable7 = VAR_PARKING_ENDING_VEHICLES_IDS;
		TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_SIM_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4 << variable5 << variable6 << variable7);
		processSubcriptionResult(buf);
		ASSERT(buf.eof());
	}

	{
		// subscribe to list of vehicle ids
		uint32_t beginTime = 0;
		uint32_t endTime = 0x7FFFFFFF;
		std::string objectId = "";
		uint8_t variableNumber = 1;
		uint8_t variable1 = ID_LIST;
		TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1);
		processSubcriptionResult(buf);
		ASSERT(buf.eof());
	}

	ObstacleControl* obstacles = ObstacleControlAccess().getIfExists();
	if (obstacles) {
		{
			// get list of polygons
			std::list<std::string> ids = getCommandInterface()->getPolygonIds();
			for (std::list<std::string>::iterator i = ids.begin(); i != ids.end(); ++i) {
				std::string id = *i;
				std::string typeId = getCommandInterface()->polygon(id).getTypeId();
				if (typeId == "building") {
					std::list<Coord> coords = getCommandInterface()->polygon(id).getShape();
					Obstacle obs(id, 9, .4); // each building gets attenuation of 9 dB per wall, 0.4 dB per meter
					std::vector<Coord> shape;
					std::copy(coords.begin(), coords.end(), std::back_inserter(shape));
					obs.setShape(shape);
					obstacles->add(obs);

				}
			}
		}
	}
}

void TraCIScenarioManager::finish() {
	if (connection) {
		TraCIBuffer buf = connection->query(CMD_CLOSE, TraCIBuffer());
	}
	while (hosts.begin() != hosts.end()) {
		deleteManagedModule(hosts.begin()->first);
	}
}

void TraCIScenarioManager::handleMessage(cMessage *msg) {
	if (msg->isSelfMessage()) {
		handleSelfMsg(msg);
		return;
	}
	error("TraCIScenarioManager doesn't handle messages from other modules");
}

void TraCIScenarioManager::handleSelfMsg(cMessage *msg) {
	if (msg == connectAndStartTrigger) {
		connection = TraCIConnection::connect(host.c_str(), port);
		commandIfc = new TraCICommandInterface(*connection);
		init_traci();
		return;
	}
	if (msg == executeOneTimestepTrigger) {
		if (simTime() > 1) {
			if (vehicleTypeIds.size()==0) {
				std::list<std::string> vehTypes = getCommandInterface()->getVehicleTypeIds();
				for (std::list<std::string>::const_iterator i = vehTypes.begin(); i != vehTypes.end(); ++i) {
					if (i->compare("DEFAULT_VEHTYPE")!=0) {
						MYDEBUG << *i << std::endl;
						vehicleTypeIds.push_back(*i);
					}
				}
			}
			if (routeIds.size()==0) {
				std::list<std::string> routes = getCommandInterface()->getRouteIds();
				for (std::list<std::string>::const_iterator i = routes.begin(); i != routes.end(); ++i) {
					std::string routeId = *i;
					if (par("useRouteDistributions").boolValue() == true) {
						if (std::count(routeId.begin(), routeId.end(), '#') >= 1) {
							MYDEBUG << "Omitting route " << routeId << " as it seems to be a member of a route distribution (found '#' in name)" << std::endl;
							continue;
						}
					}
					MYDEBUG << "Adding " << routeId << " to list of possible routes" << std::endl;
					routeIds.push_back(routeId);
				}
			}
			for (int i = activeVehicleCount + queuedVehicles.size(); i< numVehicles; i++) {
				insertNewVehicle();
			}
		}
		executeOneTimestep();
		return;
	}
	error("TraCIScenarioManager received unknown self-message");
}



// name: host;Car;i=vehicle.gif
void TraCIScenarioManager::addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id, double speed, double angle) {

	if (hosts.find(nodeId) != hosts.end()) error("tried adding duplicate module");

	if (queuedVehicles.find(nodeId) != queuedVehicles.end()) {
		queuedVehicles.erase(nodeId);
	}
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

	//TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
	cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
	mod->finalizeParameters();
	mod->getDisplayString().parse(displayString.c_str());
	mod->buildInside();
	mod->scheduleStart(simTime() + updateInterval);

	// pre-initialize TraCIMobility
	for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
		cModule* submod = iter();
		ifInetTraCIMobilityCallPreInitialize(submod, nodeId, position, road_id, speed, angle);
		TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
		if (!mm) continue;
		mm->preInitialize(nodeId, position, road_id, speed, angle);
	}

	mod->callInitialize();
	hosts[nodeId] = mod;

	// post-initialize TraCIMobility
	for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
		cModule* submod = iter();
		TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
		if (!mm) continue;
		mm->changePosition();
	}
}

cModule* TraCIScenarioManager::getManagedModule(std::string nodeId) {
	if (hosts.find(nodeId) == hosts.end()) return 0;
	return hosts[nodeId];
}

bool TraCIScenarioManager::isModuleUnequipped(std::string nodeId) {
	if (unEquippedHosts.find(nodeId) == unEquippedHosts.end()) return false;
	return true;
}

void TraCIScenarioManager::deleteManagedModule(std::string nodeId) {
	cModule* mod = getManagedModule(nodeId);
	if (!mod) error("no vehicle with Id \"%s\" found", nodeId.c_str());

	cc->unregisterNic(mod->getSubmodule("nic"));

	hosts.erase(nodeId);
	mod->callFinish();
	mod->deleteModule();
}

bool TraCIScenarioManager::isInRegionOfInterest(const TraCICoord& position, std::string road_id, double speed, double angle) {
	if ((roiRoads.size() == 0) && (roiRects.size() == 0)) return true;
	if (roiRoads.size() > 0) {
		for (std::list<std::string>::const_iterator i = roiRoads.begin(); i != roiRoads.end(); ++i) {
			if (road_id == *i) return true;
		}
	}
	if (roiRects.size() > 0) {
		for (std::list<std::pair<TraCICoord, TraCICoord> >::const_iterator i = roiRects.begin(); i != roiRects.end(); ++i) {
			if ((position.x >= i->first.x) && (position.y >= i->first.y) && (position.x <= i->second.x) && (position.y <= i->second.y)) return true;
		}
	}
	return false;
}

uint32_t TraCIScenarioManager::getCurrentTimeMs() {
	return static_cast<uint32_t>(round(simTime().dbl() * 1000));
}

void TraCIScenarioManager::executeOneTimestep() {

	MYDEBUG << "Triggering TraCI server simulation advance to t=" << simTime() <<endl;

	uint32_t targetTime = getCurrentTimeMs();

	if (targetTime > round(connectAt.dbl() * 1000)) {
		insertVehicles();
		TraCIBuffer buf = connection->query(CMD_SIMSTEP2, TraCIBuffer() << targetTime);

		uint32_t count; buf >> count;
		MYDEBUG << "Getting " << count << " subscription results" << endl;
		for (uint32_t i = 0; i < count; ++i) {
			processSubcriptionResult(buf);
		}
	}

	if (!autoShutdownTriggered) scheduleAt(simTime()+updateInterval, executeOneTimestepTrigger);

}

void TraCIScenarioManager::insertNewVehicle() {
	std::string type;
	if (vehicleTypeIds.size()) {
		int vehTypeId = mobRng->intRand(vehicleTypeIds.size());
		type = vehicleTypeIds[vehTypeId];
	}
	else {
		type = "DEFAULT_VEHTYPE";
	}
	int routeId = mobRng->intRand(routeIds.size());
	vehicleInsertQueue[routeId].push(type);
}

void TraCIScenarioManager::insertVehicles() {

	for (std::map<int, std::queue<std::string> >::iterator i = vehicleInsertQueue.begin(); i != vehicleInsertQueue.end(); ) {
		std::string route = routeIds[i->first];
		MYDEBUG << "process " << route << std::endl;
		std::queue<std::string> vehicles = i->second;
		while (!i->second.empty()) {
			bool suc = false;
			std::string type = i->second.front();
			std::stringstream veh;
			veh << type << "_" << vehicleNameCounter;
			MYDEBUG << "trying to add " << veh.str() << " with " << route << " vehicle type " << type << std::endl;

			suc = getCommandInterface()->addVehicle(veh.str(), type, route, simTime());
			if (!suc) {
				i->second.pop();
			}
			else {
				MYDEBUG << "successful inserted " << veh.str() << std::endl;
				queuedVehicles.insert(veh.str());
				i->second.pop();
				vehicleNameCounter++;
			}
		}
		std::map<int, std::queue<std::string> >::iterator tmp = i;
		++tmp;
		vehicleInsertQueue.erase(i);
		i = tmp;

	}
}

void TraCIScenarioManager::subscribeToVehicleVariables(std::string vehicleId) {
	// subscribe to some attributes of the vehicle
	uint32_t beginTime = 0;
	uint32_t endTime = 0x7FFFFFFF;
	std::string objectId = vehicleId;
	uint8_t variableNumber = 5;
	uint8_t variable1 = VAR_POSITION;
	uint8_t variable2 = VAR_ROAD_ID;
	uint8_t variable3 = VAR_SPEED;
	uint8_t variable4 = VAR_ANGLE;
	uint8_t variable5 = VAR_SIGNALS;

	TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4 << variable5);
	processSubcriptionResult(buf);
	ASSERT(buf.eof());
}

void TraCIScenarioManager::unsubscribeFromVehicleVariables(std::string vehicleId) {
	// subscribe to some attributes of the vehicle
	uint32_t beginTime = 0;
	uint32_t endTime = 0x7FFFFFFF;
	std::string objectId = vehicleId;
	uint8_t variableNumber = 0;

	TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber);
	ASSERT(buf.eof());
}

void TraCIScenarioManager::processSimSubscription(std::string objectId, TraCIBuffer& buf) {
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
				cModule* mod = getManagedModule(idstring);
				if (mod) deleteManagedModule(idstring);

				if(unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
					unEquippedHosts.erase(idstring);
				}

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
				cModule* mod = getManagedModule(idstring);
				if (mod) deleteManagedModule(idstring);

				if(unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
					unEquippedHosts.erase(idstring);
				}

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


				cModule* mod = getManagedModule(idstring);
				for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
					cModule* submod = iter();
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

				cModule* mod = getManagedModule(idstring);
				for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
					cModule* submod = iter();
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
			uint32_t omnetTimestep = getCurrentTimeMs();
			ASSERT(omnetTimestep == serverTimestep);

		} else {
			error("Received unhandled sim subscription result");
		}
	}
}

void TraCIScenarioManager::processVehicleSubscription(std::string objectId, TraCIBuffer& buf) {
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
			ASSERT(count == drivingVehicleCount);
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

	Coord p = connection->traci2omnet(TraCICoord(px, py));
	if ((p.x < 0) || (p.y < 0)) error("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);

	double angle = connection->traci2omnetAngle(angle_traci);

	cModule* mod = getManagedModule(objectId);

	// is it in the ROI?
	bool inRoi = isInRegionOfInterest(TraCICoord(px, py), edge, speed, angle);
	if (!inRoi) {
		if (mod) {
			deleteManagedModule(objectId);
			MYDEBUG << "Vehicle #" << objectId << " left region of interest" << endl;
		}
		else if(unEquippedHosts.find(objectId) != unEquippedHosts.end()) {
			unEquippedHosts.erase(objectId);
			MYDEBUG << "Vehicle (unequipped) # " << objectId<< " left region of interest" << endl;
		}
		return;
	}

	if (isModuleUnequipped(objectId)) {
		return;
	}

	if (!mod) {
		// no such module - need to create
		addModule(objectId, moduleType, moduleName, moduleDisplayString, p, edge, speed, angle);
		MYDEBUG << "Added vehicle #" << objectId << endl;
	} else {
		// module existed - update position
		for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
			cModule* submod = iter();
			ifInetTraCIMobilityCallNextPosition(submod, p, edge, speed, angle);
			TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
			if (!mm) continue;
			MYDEBUG << "module " << objectId << " moving to " << p.x << "," << p.y << endl;
			mm->nextPosition(p, edge, speed, angle);
		}
	}

}

void TraCIScenarioManager::processSubcriptionResult(TraCIBuffer& buf) {
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

