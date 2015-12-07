#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIScenarioManagerBase.h"
#include "veins/modules/mobility/traci/TraCIObstacleListener.h"
#include "veins/modules/obstacle/ObstacleControl.h"

namespace Veins
{

TraCIObstacleListener::TraCIObstacleListener(TraCIScenarioManagerBase* manager_) :
	manager(manager_)
{
}

void TraCIObstacleListener::init()
{
	TraCICommandInterface* commandIfc = manager->getCommandInterface();
	ObstacleControl* obstacles = ObstacleControlAccess().getIfExists();
	if (obstacles) {
		// get list of polygons
		std::list<std::string> ids = commandIfc->getPolygonIds();
		for (std::list<std::string>::iterator i = ids.begin(); i != ids.end(); ++i) {
			std::string id = *i;
			std::string typeId = commandIfc->polygon(id).getTypeId();
			if (!obstacles->isTypeSupported(typeId)) continue;
			std::list<Coord> coords = commandIfc->polygon(id).getShape();
			std::vector<Coord> shape;
			std::copy(coords.begin(), coords.end(), std::back_inserter(shape));
			obstacles->addFromTypeAndShape(id, typeId, shape);
		}
	}
}

} // namespace Veins
