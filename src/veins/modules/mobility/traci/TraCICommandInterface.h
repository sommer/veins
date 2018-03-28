#ifndef VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_
#define VEINS_MOBILITY_TRACI_TRACICOMMANDINTERFACE_H_

#include <list>
#include <string>
#include <stdint.h>

#include "veins/modules/mobility/traci/TraCIColor.h"
#include "veins/base/utils/Coord.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightProgram.h"

namespace Veins {

class TraCIConnection;

class TraCICommandInterface
{
	public:
		TraCICommandInterface(TraCIConnection&);

		enum DepartTime {
			DEPART_TIME_TRIGGERED = -1,
			DEPART_TIME_CONTAINER_TRIGGERED = -2,
			DEPART_TIME_NOW = -3, // Not yet documented and fully implemented (Sumo 0.30.0) 
		};

		enum DepartSpeed {
			DEPART_SPEED_RANDOM = -2,
			DEPART_SPEED_MAX = -3,
		};

		enum DepartPosition {
			DEPART_POSITION_RANDOM = -2,
			DEPART_POSITION_FREE = -3,
			DEPART_POSITION_BASE = -4,
			DEPART_POSITION_LAST = -5,
			DEPART_POSITION_RANDOM_FREE = -6,
		};

		enum DepartLane {
			DEPART_LANE_RANDOM = -2, // A random lane
			DEPART_LANE_FREE = -3, // The least occupied lane
			DEPART_LANE_ALLOWED = -4, // The least occupied lane which allows continuation
			DEPART_LANE_BEST = -5, // The least occupied of the best lanes
			DEPART_LANE_FIRST = -6, // The rightmost valid 
		};

		// General methods that do not deal with a particular object in the simulation
		std::pair<uint32_t, std::string> getVersion();
		std::pair<double, double> getLonLat(const Coord&);
		double getDistance(const Coord& position1, const Coord& position2, bool returnDrivingDistance);

		// Vehicle methods
		/**
		 * @brief Adds a vehicle to the simulation.
		 *
		 * @param vehicleId The new vehicle's ID.
		 * @param vehicleTypeId The new vehicle's type identifier.
		 * @param routeId Identifier of the new vehicle's route.
		 * @param emitTime_st Time at which to spawn the new vehicle or a value from DepartTime.
		 * @param emitPosition Position of the new vehicle on its lane. Valid values are between 0 and 1 (start and
		 * 					   end of edge) and special values from DepartPosition. 
		 * @param emitSpeed Speed in meters per second of the new vehicle. Also accepts special values from DepartSpeed.
		 * @param emitLane The new vehicle's lane. Special Also accepts special values from DepartLane.
		 * @return Success indication
		 */
		bool addVehicle(std::string vehicleId, std::string vehicleTypeId, std::string routeId, simtime_t emitTime_st=DEPART_TIME_TRIGGERED, double emitPosition=DEPART_POSITION_BASE, double emitSpeed=DEPART_SPEED_MAX, int8_t emitLane=DEPART_LANE_BEST);
		class Vehicle {
			public:
				Vehicle(TraCICommandInterface* traci, std::string nodeId) : traci(traci), nodeId(nodeId) {
					connection = &traci->connection;
				}

				void setSpeedMode(int32_t bitset);
				void setSpeed(double speed);
				void setMaxSpeed(double speed);
				void setColor(const TraCIColor& color);
				void slowDown(double speed, int time);
				void newRoute(std::string roadId);
				void setParking();
				std::string getRoadId();
				std::string getCurrentRoadOnRoute();
				std::string getLaneId();
				double getMaxSpeed();
				double getLanePosition();
				std::list<std::string> getPlannedRoadIds();
				std::string getRouteId();
				void changeRoute(std::string roadId, double travelTime);
				void stopAt(std::string roadId, double pos, uint8_t laneid, double radius, double waittime);
				int32_t getLaneIndex();
				std::string getTypeId();
				bool changeVehicleRoute(const std::list<std::string>& roads);
				double getLength();

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
		std::list<std::string> getTrafficlightIds();
		class Trafficlight {
			public:
				Trafficlight(TraCICommandInterface* traci, std::string trafficLightId) : traci(traci), trafficLightId(trafficLightId) {
					connection = &traci->connection;
				}

				std::string getCurrentState() const;
				int32_t getDefaultCurrentPhaseDuration() const;
				std::list<std::string> getControlledLanes() const;
				std::list<std::list<TraCITrafficLightLink> > getControlledLinks() const;
				int32_t getCurrentPhaseIndex() const;
				std::string getCurrentProgramID() const;
				TraCITrafficLightProgram getProgramDefinition() const;
				int32_t getAssumedNextSwitchTime() const;

				void setProgram(std::string program);/**< set/switch to different program */
				void setPhaseIndex(int32_t index); /**< set/switch to different phase within the program  */
				void setState(std::string state);
				void setPhaseDuration(int32_t duration); /**< set remaining duration of current phase in milliseconds */
				void setProgramDefinition(TraCITrafficLightProgram::Logic program, int32_t programNr);

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

				/**
				 * Track the vehicle identified by vehicleId in the Sumo GUI.
				 */
				void trackVehicle(std::string vehicleId);

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
