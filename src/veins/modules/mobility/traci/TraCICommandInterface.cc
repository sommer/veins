#include <stdlib.h>

#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/TraCICoord.h"

#ifdef _WIN32
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#endif /* _WIN32 */

namespace Veins {

TraCICommandInterface::TraCICommandInterface(TraCIConnection& c) : connection(c)
{
}

std::pair<uint32_t, std::string> TraCICommandInterface::getVersion() {
	bool success = false;
	TraCIBuffer buf = connection.queryOptional(CMD_GETVERSION, TraCIBuffer(), success);

	if (!success) {
		ASSERT(buf.eof());
		return std::pair<uint32_t, std::string>(0, "(unknown)");
	}

	uint8_t cmdLength; buf >> cmdLength;
	uint8_t commandResp; buf >> commandResp;
	ASSERT(commandResp == CMD_GETVERSION);
	uint32_t apiVersion; buf >> apiVersion;
	std::string serverVersion; buf >> serverVersion;
	ASSERT(buf.eof());

	return std::pair<uint32_t, std::string>(apiVersion, serverVersion);
}

void TraCICommandInterface::Vehicle::setSpeedMode(int32_t bitset) {
	uint8_t variableId = VAR_SPEEDSETMODE;
	uint8_t variableType = TYPE_INTEGER;
	TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << bitset);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::setSpeed(double speed) {
	uint8_t variableId = VAR_SPEED;
	uint8_t variableType = TYPE_DOUBLE;
	TraCIBuffer buf = traci->connection.query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << speed);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::setColor(const TraCIColor& color) {
	TraCIBuffer p;
	p << static_cast<uint8_t>(VAR_COLOR);
	p << nodeId;
	p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
	TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, p);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::slowDown(double speed, int32_t time) {
	uint8_t variableId = CMD_SLOWDOWN;
	uint8_t variableType = TYPE_COMPOUND;
	int32_t count = 2;
	uint8_t speedType = TYPE_DOUBLE;
	uint8_t durationType = TYPE_INTEGER;
	TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << speedType << speed << durationType << time);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::newRoute(std::string roadId) {
	uint8_t variableId = LANE_EDGE_ID;
	uint8_t variableType = TYPE_STRING;
	TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << roadId);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Vehicle::setParking() {
	uint8_t variableId = REMOVE;
	uint8_t variableType = TYPE_BYTE;
	uint8_t value = REMOVE_PARKING;
	TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << value);
	ASSERT(buf.eof());
}

std::list<std::string> TraCICommandInterface::getVehicleTypeIds() {
	return genericGetStringList(CMD_GET_VEHICLETYPE_VARIABLE, "", ID_LIST, RESPONSE_GET_VEHICLETYPE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::getRouteIds() {
	return genericGetStringList(CMD_GET_ROUTE_VARIABLE, "", ID_LIST, RESPONSE_GET_ROUTE_VARIABLE);
}

double TraCICommandInterface::Road::getCurrentTravelTime() {
	return traci->genericGetDouble(CMD_GET_EDGE_VARIABLE, roadId, VAR_CURRENT_TRAVELTIME, RESPONSE_GET_EDGE_VARIABLE);
}

double TraCICommandInterface::Road::getMeanSpeed() {
	return traci->genericGetDouble(CMD_GET_EDGE_VARIABLE, roadId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_EDGE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getRoadId() {
	return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROAD_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getCurrentRoadOnRoute() {
	return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, LANE_EDGE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getLaneId() {
	return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

int32_t TraCICommandInterface::Vehicle::getLaneIndex() {
	return traci->genericGetInt(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANE_INDEX, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getTypeId() {
	return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_TYPE, RESPONSE_GET_VEHICLE_VARIABLE);
}

double TraCICommandInterface::Vehicle::getLanePosition() {
	return traci->genericGetDouble(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_LANEPOSITION, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::Vehicle::getPlannedRoadIds() {
	return traci->genericGetStringList(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_EDGES, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::string TraCICommandInterface::Vehicle::getRouteId() {
	return traci->genericGetString(CMD_GET_VEHICLE_VARIABLE, nodeId, VAR_ROUTE_ID, RESPONSE_GET_VEHICLE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::Route::getRoadIds() {
	return traci->genericGetStringList(CMD_GET_ROUTE_VARIABLE, routeId, VAR_EDGES, RESPONSE_GET_ROUTE_VARIABLE);
}

void TraCICommandInterface::Vehicle::changeRoute(std::string roadId, double travelTime) {
	if (travelTime >= 0) {
		uint8_t variableId = VAR_EDGE_TRAVELTIME;
		uint8_t variableType = TYPE_COMPOUND;
		int32_t count = 2;
		uint8_t edgeIdT = TYPE_STRING;
		std::string edgeId = roadId;
		uint8_t newTimeT = TYPE_DOUBLE;
		double newTime = travelTime;
		TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId << newTimeT << newTime);
		ASSERT(buf.eof());
	} else {
		uint8_t variableId = VAR_EDGE_TRAVELTIME;
		uint8_t variableType = TYPE_COMPOUND;
		int32_t count = 1;
		uint8_t edgeIdT = TYPE_STRING;
		std::string edgeId = roadId;
		TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId);
		ASSERT(buf.eof());
	}
	{
		uint8_t variableId = CMD_REROUTE_TRAVELTIME;
		uint8_t variableType = TYPE_COMPOUND;
		int32_t count = 0;
		TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count);
		ASSERT(buf.eof());
	}
}

double TraCICommandInterface::getDistance(const Coord& p1, const Coord& p2, bool returnDrivingDistance) {
	uint8_t variable = DISTANCE_REQUEST;
	std::string simId = "sim0";
	uint8_t variableType = TYPE_COMPOUND;
	int32_t count = 3;
	uint8_t dType = static_cast<uint8_t>(returnDrivingDistance ? REQUEST_DRIVINGDIST : REQUEST_AIRDIST);

	// query road network boundaries
	TraCIBuffer buf = connection.query(CMD_GET_SIM_VARIABLE, TraCIBuffer() << variable << simId << variableType << count << connection.omnet2traci(p1) << connection.omnet2traci(p2) << dType);
	uint8_t cmdLength_resp; buf >> cmdLength_resp;
	uint8_t commandId_resp; buf >> commandId_resp; ASSERT(commandId_resp == RESPONSE_GET_SIM_VARIABLE);
	uint8_t variableId_resp; buf >> variableId_resp; ASSERT(variableId_resp == variable);
	std::string simId_resp; buf >> simId_resp; ASSERT(simId_resp == simId);
	uint8_t typeId_resp; buf >> typeId_resp; ASSERT(typeId_resp == TYPE_DOUBLE);
	double distance; buf >> distance;

	ASSERT(buf.eof());

	return distance;
}

void TraCICommandInterface::Vehicle::stopAt(std::string roadId, double pos, uint8_t laneid, double radius, double waittime) {
	uint8_t variableId = CMD_STOP;
	uint8_t variableType = TYPE_COMPOUND;
	int32_t count = 4;
	uint8_t edgeIdT = TYPE_STRING;
	std::string edgeId = roadId;
	uint8_t stopPosT = TYPE_DOUBLE;
	double stopPos = pos;
	uint8_t stopLaneT = TYPE_BYTE;
	uint8_t stopLane = laneid;
	uint8_t durationT = TYPE_INTEGER;
	uint32_t duration = waittime * 1000;

	TraCIBuffer buf = connection->query(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << nodeId << variableType << count << edgeIdT << edgeId << stopPosT << stopPos << stopLaneT << stopLane << durationT << duration);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Trafficlight::setProgram(std::string program) {
	TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PROGRAM) << trafficLightId << static_cast<uint8_t>(TYPE_STRING) << program);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Trafficlight::setPhaseIndex(int32_t index) {
	TraCIBuffer buf = connection->query(CMD_SET_TL_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(TL_PHASE_INDEX) << trafficLightId << static_cast<uint8_t>(TYPE_INTEGER) << index);
	ASSERT(buf.eof());
}

std::list<std::string> TraCICommandInterface::getPolygonIds() {
	return genericGetStringList(CMD_GET_POLYGON_VARIABLE, "", ID_LIST, RESPONSE_GET_POLYGON_VARIABLE);
}

std::string TraCICommandInterface::Polygon::getTypeId() {
	return traci->genericGetString(CMD_GET_POLYGON_VARIABLE, polyId, VAR_TYPE, RESPONSE_GET_POLYGON_VARIABLE);
}

std::list<Coord> TraCICommandInterface::Polygon::getShape() {
	return traci->genericGetCoordList(CMD_GET_POLYGON_VARIABLE, polyId, VAR_SHAPE, RESPONSE_GET_POLYGON_VARIABLE);
}

void TraCICommandInterface::Polygon::setShape(const std::list<Coord>& points) {
	TraCIBuffer buf;
	uint8_t count = static_cast<uint8_t>(points.size());
	buf << static_cast<uint8_t>(VAR_SHAPE) << polyId << static_cast<uint8_t>(TYPE_POLYGON) << count;
	for (std::list<Coord>::const_iterator i = points.begin(); i != points.end(); ++i) {
		const TraCICoord& pos = connection->omnet2traci(*i);
		buf << static_cast<double>(pos.x) << static_cast<double>(pos.y);
	}
	TraCIBuffer obuf = connection->query(CMD_SET_POLYGON_VARIABLE, buf);
	ASSERT(obuf.eof());
}

void TraCICommandInterface::addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<Coord>& points) {
	TraCIBuffer p;

	p << static_cast<uint8_t>(ADD) << polyId;
	p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(5);
	p << static_cast<uint8_t>(TYPE_STRING) << polyType;
	p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
	p << static_cast<uint8_t>(TYPE_UBYTE) << static_cast<uint8_t>(filled);
	p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
	p << static_cast<uint8_t>(TYPE_POLYGON) << static_cast<uint8_t>(points.size());
	for (std::list<Coord>::const_iterator i = points.begin(); i != points.end(); ++i) {
		const TraCICoord& pos = connection.omnet2traci(*i);
		p << static_cast<double>(pos.x) << static_cast<double>(pos.y);
	}

	TraCIBuffer buf = connection.query(CMD_SET_POLYGON_VARIABLE, p);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Polygon::remove(int32_t layer) {
	TraCIBuffer p;

	p << static_cast<uint8_t>(REMOVE) << polyId;
	p << static_cast<uint8_t>(TYPE_INTEGER) << layer;

	TraCIBuffer buf = connection->query(CMD_SET_POLYGON_VARIABLE, p);
	ASSERT(buf.eof());
}

void TraCICommandInterface::addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos_) {
	TraCIBuffer p;

	TraCICoord pos = connection.omnet2traci(pos_);
	p << static_cast<uint8_t>(ADD) << poiId;
	p << static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(4);
	p << static_cast<uint8_t>(TYPE_STRING) << poiType;
	p << static_cast<uint8_t>(TYPE_COLOR) << color.red << color.green << color.blue << color.alpha;
	p << static_cast<uint8_t>(TYPE_INTEGER) << layer;
	p << pos;

	TraCIBuffer buf = connection.query(CMD_SET_POI_VARIABLE, p);
	ASSERT(buf.eof());
}

void TraCICommandInterface::Poi::remove(int32_t layer) {
	TraCIBuffer p;

	p << static_cast<uint8_t>(REMOVE) << poiId;
	p << static_cast<uint8_t>(TYPE_INTEGER) << layer;

	TraCIBuffer buf = connection->query(CMD_SET_POI_VARIABLE, p);
	ASSERT(buf.eof());
}

std::list<std::string> TraCICommandInterface::getLaneIds() {
	return genericGetStringList(CMD_GET_LANE_VARIABLE, "", ID_LIST, RESPONSE_GET_LANE_VARIABLE);
}

std::list<Coord> TraCICommandInterface::Lane::getShape() {
	return traci->genericGetCoordList(CMD_GET_LANE_VARIABLE, laneId, VAR_SHAPE, RESPONSE_GET_LANE_VARIABLE);
}

std::string TraCICommandInterface::Lane::getRoadId() {
	return traci->genericGetString(CMD_GET_LANE_VARIABLE, laneId, LANE_EDGE_ID, RESPONSE_GET_LANE_VARIABLE);
}

double TraCICommandInterface::Lane::getLength() {
	return traci->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_LENGTH, RESPONSE_GET_LANE_VARIABLE);
}

double TraCICommandInterface::Lane::getMaxSpeed() {
	return traci->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, VAR_MAXSPEED, RESPONSE_GET_LANE_VARIABLE);
}

double TraCICommandInterface::Lane::getMeanSpeed() {
	return traci->genericGetDouble(CMD_GET_LANE_VARIABLE, laneId, LAST_STEP_MEAN_SPEED, RESPONSE_GET_LANE_VARIABLE);
}

std::list<std::string> TraCICommandInterface::getJunctionIds() {
	return genericGetStringList(CMD_GET_JUNCTION_VARIABLE, "", ID_LIST, RESPONSE_GET_JUNCTION_VARIABLE);
}

Coord TraCICommandInterface::Junction::getPosition() {
	return traci->genericGetCoord(CMD_GET_JUNCTION_VARIABLE, junctionId, VAR_POSITION, RESPONSE_GET_JUNCTION_VARIABLE);
}

bool TraCICommandInterface::addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st, double emitPosition, double emitSpeed, int8_t emitLane) {
	bool success = false;

	uint8_t variableId = ADD;
	uint8_t variableType = TYPE_COMPOUND;
	int32_t count = 6;
	int32_t emitTime = (emitTime_st < 0) ? (-1) : (floor(emitTime_st.dbl() * 1000));
	TraCIBuffer buf = connection.queryOptional(CMD_SET_VEHICLE_VARIABLE, TraCIBuffer() << variableId << vehicleId << variableType << count << (uint8_t)TYPE_STRING << vehicleTypeId << (uint8_t)TYPE_STRING << routeId << (uint8_t)TYPE_INTEGER << emitTime << (uint8_t)TYPE_DOUBLE << emitPosition << (uint8_t)TYPE_DOUBLE <<emitSpeed << (uint8_t)TYPE_BYTE << emitLane, success);
	ASSERT(buf.eof());

	return success;
}

bool TraCICommandInterface::Vehicle::changeVehicleRoute(const std::list<std::string>& edges) {
	if (getRoadId().find(':') != std::string::npos) return false;
	if (edges.front().compare(getRoadId()) != 0) return false;
	uint8_t variableId = VAR_ROUTE;
	uint8_t variableType = TYPE_STRINGLIST;
	TraCIBuffer buf;
	buf << variableId << nodeId << variableType;
	int32_t numElem = edges.size();
	buf << numElem;
	for (std::list<std::string>::const_iterator i = edges.begin(); i != edges.end(); ++i) {
		buf << static_cast<std::string>(*i);
	}
	TraCIBuffer obuf = connection->query(CMD_SET_VEHICLE_VARIABLE, buf);
	ASSERT(obuf.eof());
	return true;
}

std::pair<double, double> TraCICommandInterface::getLonLat(const Coord& coord) {
	TraCIBuffer request;
	request << static_cast<uint8_t>(POSITION_CONVERSION) << std::string("sim0")
			<< static_cast<uint8_t>(TYPE_COMPOUND) << static_cast<int32_t>(2)
			<< connection.omnet2traci(coord)
			<< static_cast<uint8_t>(TYPE_UBYTE) << static_cast<uint8_t>(POSITION_LON_LAT);
	TraCIBuffer response = connection.query(CMD_GET_SIM_VARIABLE, request);

	uint8_t cmdLength; response >> cmdLength;
	if (cmdLength == 0) {
		uint32_t cmdLengthX;
		response >> cmdLengthX;
	}
	uint8_t responseId; response >> responseId;
	ASSERT(responseId == RESPONSE_GET_SIM_VARIABLE);
	uint8_t variable; response >> variable;
	ASSERT(variable == POSITION_CONVERSION);
	std::string id; response >> id;
	uint8_t convPosType; response >> convPosType;
	ASSERT(convPosType == POSITION_LON_LAT);
	double convPosLon; response >> convPosLon;
	double convPosLat; response >> convPosLat;

	return std::make_pair(convPosLon, convPosLat);
}

void TraCICommandInterface::GuiView::setScheme(std::string name) {
	TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_VIEW_SCHEMA) << viewId << static_cast<uint8_t>(TYPE_STRING) << name);
	ASSERT(buf.eof());
}

void TraCICommandInterface::GuiView::setZoom(double zoom) {
	TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_VIEW_ZOOM) << viewId << static_cast<uint8_t>(TYPE_DOUBLE) << zoom);
	ASSERT(buf.eof());
}

void TraCICommandInterface::GuiView::setBoundary(Coord p1_, Coord p2_) {
	TraCICoord p1 = connection->omnet2traci(p1_);
	TraCICoord p2 = connection->omnet2traci(p2_);

	TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_VIEW_BOUNDARY) << viewId << static_cast<uint8_t>(TYPE_BOUNDINGBOX) << p1.x << p1.y << p2.x << p2.y);
	ASSERT(buf.eof());
}


void TraCICommandInterface::GuiView::takeScreenshot(std::string filename) {
	if (filename == "") {
		// get absolute path of results/ directory
		const char* myResultsDir = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RESULTDIR);
		char* s = realpath(myResultsDir, 0);
		std::string absolutePath = s;
		free(s);

		// get run id
		const char* myRunID = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNID);

		// build filename from this
		char ss[512];
		snprintf(ss, sizeof(ss), "%s/screenshot-%s-@%08.2f.png", absolutePath.c_str(), myRunID, simTime().dbl());
		filename = ss;
	}

	TraCIBuffer buf = connection->query(CMD_SET_GUI_VARIABLE, TraCIBuffer() << static_cast<uint8_t>(VAR_SCREENSHOT) << viewId << static_cast<uint8_t>(TYPE_STRING) << filename);
	ASSERT(buf.eof());
}

std::string TraCICommandInterface::genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId) {
	uint8_t resultTypeId = TYPE_STRING;
	std::string res;

	TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

	uint8_t cmdLength; buf >> cmdLength;
	if (cmdLength == 0) {
		uint32_t cmdLengthX;
		buf >> cmdLengthX;
	}
	uint8_t commandId_r; buf >> commandId_r;
	ASSERT(commandId_r == responseId);
	uint8_t varId; buf >> varId;
	ASSERT(varId == variableId);
	std::string objectId_r; buf >> objectId_r;
	ASSERT(objectId_r == objectId);
	uint8_t resType_r; buf >> resType_r;
	ASSERT(resType_r == resultTypeId);
	buf >> res;

	ASSERT(buf.eof());

	return res;
}

Coord TraCICommandInterface::genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId) {

	uint8_t resultTypeId = POSITION_2D;
	double x;
	double y;

	TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

	uint8_t cmdLength; buf >> cmdLength;
	if (cmdLength == 0) {
		uint32_t cmdLengthX;
		buf >> cmdLengthX;
	}
	uint8_t commandId_r; buf >> commandId_r;
	ASSERT(commandId_r == responseId);
	uint8_t varId; buf >> varId;
	ASSERT(varId == variableId);
	std::string objectId_r; buf >> objectId_r;
	ASSERT(objectId_r == objectId);
	uint8_t resType_r; buf >> resType_r;
	ASSERT(resType_r == resultTypeId);
	buf >> x;
	buf >> y;

	ASSERT(buf.eof());

	return connection.traci2omnet(TraCICoord(x, y));
}

double TraCICommandInterface::genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId) {

	uint8_t resultTypeId = TYPE_DOUBLE;
	double res;

	TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

	uint8_t cmdLength; buf >> cmdLength;
	if (cmdLength == 0) {
		uint32_t cmdLengthX;
		buf >> cmdLengthX;
	}
	uint8_t commandId_r; buf >> commandId_r;
	ASSERT(commandId_r == responseId);
	uint8_t varId; buf >> varId;
	ASSERT(varId == variableId);
	std::string objectId_r; buf >> objectId_r;
	ASSERT(objectId_r == objectId);
	uint8_t resType_r; buf >> resType_r;
	ASSERT(resType_r == resultTypeId);
	buf >> res;

	ASSERT(buf.eof());

	return res;
}

int32_t TraCICommandInterface::genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId) {

	uint8_t resultTypeId = TYPE_INTEGER;
	int32_t res;

	TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

	uint8_t cmdLength; buf >> cmdLength;
	if (cmdLength == 0) {
		uint32_t cmdLengthX;
		buf >> cmdLengthX;
	}
	uint8_t commandId_r; buf >> commandId_r;
	ASSERT(commandId_r == responseId);
	uint8_t varId; buf >> varId;
	ASSERT(varId == variableId);
	std::string objectId_r; buf >> objectId_r;
	ASSERT(objectId_r == objectId);
	uint8_t resType_r; buf >> resType_r;
	ASSERT(resType_r == resultTypeId);
	buf >> res;

	ASSERT(buf.eof());

	return res;
}

std::list<std::string> TraCICommandInterface::genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId) {

	uint8_t resultTypeId = TYPE_STRINGLIST;
	std::list<std::string> res;

	TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

	uint8_t cmdLength; buf >> cmdLength;
	if (cmdLength == 0) {
		uint32_t cmdLengthX;
		buf >> cmdLengthX;
	}
	uint8_t commandId_r; buf >> commandId_r;
	ASSERT(commandId_r == responseId);
	uint8_t varId; buf >> varId;
	ASSERT(varId == variableId);
	std::string objectId_r; buf >> objectId_r;
	ASSERT(objectId_r == objectId);
	uint8_t resType_r; buf >> resType_r;
	ASSERT(resType_r == resultTypeId);
	uint32_t count; buf >> count;
	for (uint32_t i = 0; i < count; i++) {
		std::string id; buf >> id;
		res.push_back(id);
	}

	ASSERT(buf.eof());

	return res;
}

std::list<Coord> TraCICommandInterface::genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId) {

	uint8_t resultTypeId = TYPE_POLYGON;
	std::list<Coord> res;

	TraCIBuffer buf = connection.query(commandId, TraCIBuffer() << variableId << objectId);

	uint8_t cmdLength; buf >> cmdLength;
	if (cmdLength == 0) {
		uint32_t cmdLengthX;
		buf >> cmdLengthX;
	}
	uint8_t commandId_r; buf >> commandId_r;
	ASSERT(commandId_r == responseId);
	uint8_t varId; buf >> varId;
	ASSERT(varId == variableId);
	std::string objectId_r; buf >> objectId_r;
	ASSERT(objectId_r == objectId);
	uint8_t resType_r; buf >> resType_r;
	ASSERT(resType_r == resultTypeId);
	uint8_t count; buf >> count;
	for (uint32_t i = 0; i < count; i++) {
		double x; buf >> x;
		double y; buf >> y;
		res.push_back(connection.traci2omnet(TraCICoord(x, y)));
	}

	ASSERT(buf.eof());

	return res;
}

}
