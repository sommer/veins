#ifndef VEINS_WORLD_TRACI_TRACIVEHICLEINSERTER_H
#define VEINS_WORLD_TRACI_TRACIVEHICLEINSERTER_H

#include <set>
#include <string>
#include <vector>
#include <omnetpp.h>

namespace Veins {

class TraCICommandInterface;

class TraCIVehicleInserter
{
	public:
		TraCIVehicleInserter(TraCICommandInterface*, cRNG*);
		void enqueueVehicles(unsigned currentVehicles, simtime_t now);
		void dequeVehicle(const std::string&);
		void setTargetVehicles(unsigned number) { targetVehicles = number; }
		void useRouteDistributions(bool flag) { routeDistributions = flag; }

	private:
		void retrieveVehicleTypes();
		void retrieveRoutes();
		std::string determineVehicleType();
		std::string determineRoute();

		TraCICommandInterface* cmd;
		cRNG* rng;
		unsigned targetVehicles;
		unsigned vehicleNameCounter;
		bool routeDistributions;
		std::vector<std::string> vehicleTypeIds;
		std::vector<std::string> routeIds;
		std::set<std::string> vehicleQueue;
};

} // namespace Veins

#endif
