#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIScenarioManagerBase.h"
#include "veins/modules/mobility/traci/TraCIWorldListener.h"

namespace Veins
{

TraCIWorldListener::TraCIWorldListener(TraCIScenarioManagerBase* manager_, BaseWorldUtility* world_) :
    manager(manager_), world(world_)
{
}

void TraCIWorldListener::init()
{
	TraCICommandInterface* commandIfc = manager->getCommandInterface();
	if (commandIfc->netBoundary().within(*world->getPgs())) {
		const Coord& netbounds = commandIfc->netBoundary().max();
		EV << "WARNING: Playground size (" << world->getPgs()->x << ", " << world->getPgs()->y
			<< ") might be too small for vehicle at network bounds ("
			<< netbounds.x << ", " << netbounds.y << ")" << endl;
	}
}

} // namespace Veins
