#ifndef VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_
#define VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_

#include <list>
#include <string>
#include <stdint.h>

#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/base/utils/Coord.h"

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

		// General methods that do not deal with a particular object in the simulation
		std::pair<uint32_t, std::string> getVersion();
		std::pair<double, double> getLonLat(const Coord&);
		double getDistance(const Coord& position1, const Coord& position2, bool returnDrivingDistance);

		// Vehicle methods
		bool addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st = -DEPART_NOW, double emitPosition = -DEPART_POS_BASE, double emitSpeed = -DEPART_SPEED_MAX, int8_t emitLane = -DEPART_LANE_BEST_FREE);
		class Vehicle {
			public:
				Vehicle(TraCICommandInterface* traci, std::string nodeId) : traci(traci), nodeId(nodeId) {
					connection = &traci->connection;
				}

				void setSpeedMode(int32_t bitset);
				void setSpeed(double speed);
				void setColor(const TraCIColor& color);
				void slowDown(double speed, int time);
				void newRoute(std::string roadId);
				void setParking();
				std::string getRoadId();
				std::string getCurrentRoadOnRoute();
				std::string getLaneId();
				double getLanePosition();
				std::list<std::string> getPlannedRoadIds();
				std::string getRouteId();
				void changeRoute(std::string roadId, double travelTime);
				void stopAt(std::string roadId, double pos, uint8_t laneid, double radius, double waittime);
				int32_t getLaneIndex();
				std::string getTypeId();
				bool changeVehicleRoute(const std::list<std::string>& roads);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string nodeId;
		};
		Vehicle vehicle(std::string nodeId) {
			return Vehicle(this, nodeId);
		}

		// Road methods
		class Road {
			public:
				Road(TraCICommandInterface* traci, std::string roadId) : traci(traci), roadId(roadId) {
					connection = &traci->connection;
				}

				double getCurrentTravelTime();
				double getMeanSpeed();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string roadId;
		};
		Road road(std::string roadId) {
			return Road(this, roadId);
		}

		// Lane methods
		std::list<std::string> getLaneIds();
		class Lane {
			public:
				Lane(TraCICommandInterface* traci, std::string laneId) : traci(traci), laneId(laneId) {
					connection = &traci->connection;
				}

				std::list<Coord> getShape();
				std::string getRoadId();
				double getLength();
				double getMaxSpeed();
				double getMeanSpeed();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string laneId;
		};
		Lane lane(std::string laneId) {
			return Lane(this, laneId);
		}

		// Trafficlight methods
		class Trafficlight {
			public:
				Trafficlight(TraCICommandInterface* traci, std::string trafficLightId) : traci(traci), trafficLightId(trafficLightId) {
					connection = &traci->connection;
				}

				void setProgram(std::string program);
				void setPhaseIndex(int32_t index);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string trafficLightId;
		};
		Trafficlight trafficlight(std::string trafficLightId) {
			return Trafficlight(this, trafficLightId);
		}

		// Polygon methods
		std::list<std::string> getPolygonIds();
		void addPolygon(std::string polyId, std::string polyType, const TraCIColor& color, bool filled, int32_t layer, const std::list<Coord>& points);
		class Polygon {
			public:
				Polygon(TraCICommandInterface* traci, std::string polyId) : traci(traci), polyId(polyId) {
					connection = &traci->connection;
				}

				std::string getTypeId();
				std::list<Coord> getShape();
				void setShape(const std::list<Coord>& points);
				void remove(int32_t layer);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string polyId;
		};
		Polygon polygon(std::string polyId) {
			return Polygon(this, polyId);
		}

		// Poi methods
		void addPoi(std::string poiId, std::string poiType, const TraCIColor& color, int32_t layer, const Coord& pos);
		class Poi {
			public:
				Poi(TraCICommandInterface* traci, std::string poiId) : traci(traci), poiId(poiId) {
					connection = &traci->connection;
				}

				void remove(int32_t layer);

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string poiId;
		};
		Poi poi(std::string poiId) {
			return Poi(this, poiId);
		}

		// Junction methods
		std::list<std::string> getJunctionIds();
		class Junction {
			public:
				Junction(TraCICommandInterface* traci, std::string junctionId) : traci(traci), junctionId(junctionId) {
					connection = &traci->connection;
				}

				Coord getPosition();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string junctionId;
		};
		Junction junction(std::string junctionId) {
			return Junction(this, junctionId);
		}

		// Route methods
		std::list<std::string> getRouteIds();
		class Route {
			public:
				Route(TraCICommandInterface* traci, std::string routeId) : traci(traci), routeId(routeId) {
					connection = &traci->connection;
				}

				std::list<std::string> getRoadIds();

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string routeId;
		};
		Route route(std::string routeId) {
			return Route(this, routeId);
		}

		// Vehicletype methods
		std::list<std::string> getVehicleTypeIds();

		// GuiView methods
		class GuiView {
			public:
				GuiView(TraCICommandInterface* traci, std::string viewId) : traci(traci), viewId(viewId) {
					connection = &traci->connection;
				}

				void setScheme(std::string name);
				void setZoom(double zoom);
				void setBoundary(Coord p1, Coord p2);
				void takeScreenshot(std::string filename = "");

			protected:
				TraCICommandInterface* traci;
				TraCIConnection* connection;
				std::string viewId;
		};
		GuiView guiView(std::string viewId) {
			return GuiView(this, viewId);
		}


	private:
		TraCIConnection& connection;

		std::string genericGetString(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		Coord genericGetCoord(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		double genericGetDouble(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		int32_t genericGetInt(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<std::string> genericGetStringList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
		std::list<Coord> genericGetCoordList(uint8_t commandId, std::string objectId, uint8_t variableId, uint8_t responseId);
};

}

#endif
