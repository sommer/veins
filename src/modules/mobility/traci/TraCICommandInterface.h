#ifndef VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_
#define VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_

#include <list>
#include <string>
#include <stdint.h>

#include "modules/mobility/traci/TraCIColor.h"
#include "modules/mobility/traci/TraCICoord.h"

namespace Veins {

class TraCIConnection;

class TraCICommandInterface
{
	public:
		TraCICommandInterface(TraCIConnection&);

		enum DepartDefs {
			DEPART_NOW = 2,
			DEPART_LANE_BEST_FREE = 5,
			DEPART_POS_BASE = 4,
			DEPART_SPEED_MAX = 3
		};

		std::pair<uint32_t, std::string> getVersion();
		void setSpeedMode(std::string nodeId, int32_t bitset);
		void setSpeed(std::string nodeId, double speed);
		void setColor(std::string nodeId, const TraCIColor& color);
		void slowDown(std::string nodeId, double speed, int time);
		void newRoute(std::string nodeId, std::string roadId);
		void setVehicleParking(std::string nodeId);
		double getEdgeCurrentTravelTime(std::string edgeId) ;
		double getEdgeMeanSpeed(std::string edgeId) ;
		std::string getEdgeId(std::string nodeId);
		std::string getCurrentEdgeOnRoute(std::string nodeId);
		std::string getLaneId(std::string nodeId);
		double getLanePosition(std::string nodeId);
		std::list<std::string> getPlannedEdgeIds(std::string nodeId);
		std::string getRouteId(std::string nodeId);
		std::list<std::string> getRouteEdgeIds(std::string routeId);
		void changeRoute(std::string nodeId, std::string roadId, double travelTime);
		double distanceRequest(const TraCICoord& position1, const TraCICoord& position2, bool returnDrivingDistance);
		void stopNode(std::string nodeId, std::string roadId, double pos, uint8_t laneid, double radius, double waittime);
		void setTrafficLightProgram(std::string trafficLightId, std::string program);
		void setTrafficLightPhaseIndex(std::string trafficLightId, int32_t index);
		std::list<std::string> getPolygonIds();
		std::string getPolygonTypeId(std::string polyId);
		std::list<TraCICoord> getPolygonShape(std::string polyId);
		void setPolygonShape(std::string polyId, const std::list<TraCICoord>& points);
		void addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<TraCICoord>& points);
		void removePolygon(std::string polyId, int32_t layer);
		void addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const TraCICoord& pos);
		void removePoi(std::string poiId, int32_t layer);
		std::list<std::string> getLaneIds();
		std::list<TraCICoord> getLaneShape(std::string laneId);
		std::string getLaneEdgeId(std::string laneId);
		double getLaneLength(std::string laneId);
		double getLaneMaxSpeed(std::string laneId);
		double getLaneMeanSpeed(std::string laneId);
		int32_t getLaneIndex(std::string nodeId);
		std::list<std::string> getJunctionIds();
		TraCICoord getJunctionPosition(std::string junctionId);
		bool addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st = -DEPART_NOW, double emitPosition = -DEPART_POS_BASE, double emitSpeed = -DEPART_SPEED_MAX, int8_t emitLane = -DEPART_LANE_BEST_FREE);
		std::string getVehicleTypeId(std::string nodeId);
		std::list<std::string> getVehicleTypeIds();
		std::list<std::string> getRouteIds();
		bool changeVehicleRoute(std::string nodeId, const std::list<std::string>& edges);
		std::pair<double, double> positionConversionLonLat(const TraCICoord&);

	private:
		TraCIConnection& connection;

		std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		TraCICoord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		int32_t genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<std::string> genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<TraCICoord> genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
};

}

#endif
