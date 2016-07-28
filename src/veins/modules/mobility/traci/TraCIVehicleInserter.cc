#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIVehicleInserter.h"
#include <sstream>

namespace Veins {

TraCIVehicleInserter::TraCIVehicleInserter(TraCICommandInterface* _cmd, cRNG* _rng) :
	cmd(_cmd), rng(_rng), targetVehicles(0), vehicleNameCounter(0), routeDistributions(false)
{
	ASSERT(cmd != NULL);
	ASSERT(rng != NULL);
	retrieveVehicleTypes();
	retrieveRoutes();
}

void TraCIVehicleInserter::retrieveVehicleTypes()
{
	std::list<std::string> types = cmd->getVehicleTypeIds();
	types.remove("DEFAULT_VEHTYPE");
	types.remove("DEFAULT_PEDTYPE");
	vehicleTypeIds.assign(types.begin(), types.end());

	typedef std::vector<std::string>::const_iterator iter_t;
	for (iter_t it = vehicleTypeIds.begin(); it != vehicleTypeIds.end(); ++it) {
		EV << *it << std::endl;
	}
}

void TraCIVehicleInserter::retrieveRoutes()
{
	std::list<std::string> routes = cmd->getRouteIds();
	typedef std::list<std::string>::const_iterator iter_t;
	for (iter_t it = routes.begin(); it != routes.end(); ++it) {
		const std::string& routeId = *it;
		if (!routeDistributions && routeId.find('#') != std::string::npos) {
			EV << "Omitting route " << routeId << " as it seems to be a member "
				<< "of a route distribution (found '#' in name)" << std::endl;
			continue;
		}
		EV << "Adding " << routeId << " to list of possible routes" << std::endl;
		routeIds.push_back(routeId);
	}
}

std::string TraCIVehicleInserter::determineVehicleType()
{
	std::string type;
	if (!vehicleTypeIds.empty()) {
		const int idx = rng->intRand(vehicleTypeIds.size());
		type = vehicleTypeIds[idx];
	} else {
		type = "DEFAULT_VEHTYPE";
	}
	return type;
}

std::string TraCIVehicleInserter::determineRoute()
{
	ASSERT(!routeIds.empty());
	const int idx = rng->intRand(routeIds.size());
	return routeIds[idx];
}

void TraCIVehicleInserter::enqueueVehicles(unsigned currentVehicles, simtime_t now)
{
	currentVehicles += vehicleQueue.size();
	for (unsigned i = currentVehicles; i < targetVehicles; ++i) {
		const std::string route = determineRoute();
		const std::string type = determineVehicleType();
		std::stringstream name;
		name << type << "_" << vehicleNameCounter;
		EV << "trying to add " << name.str() << " with " << route << " vehicle type " << type << std::endl;

		if (cmd->addVehicle(name.str(), type, route, now)) {
			EV << "successfully inserted " << name.str() << std::endl;
			vehicleQueue.insert(name.str());
			++vehicleNameCounter;
		}
	}
}

void TraCIVehicleInserter::dequeVehicle(const std::string& node)
{
	vehicleQueue.erase(node);
}

} // namespace Veins
