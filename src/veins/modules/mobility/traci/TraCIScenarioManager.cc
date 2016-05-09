#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/modules/mobility/traci/TraCIObstacleListener.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCIVeinsNodeManager.h"
#include "veins/modules/mobility/traci/TraCIWorldListener.h"

Define_Module(Veins::TraCIScenarioManager);

namespace Veins {

TraCIScenarioManager::TraCIScenarioManager() :
	worldListener(0), obstacleListener(0), nodeManager(0)
{
}

TraCIScenarioManager::~TraCIScenarioManager()
{
	delete(worldListener);
	delete(obstacleListener);
	delete(nodeManager);
}

void TraCIScenarioManager::initialize(int stage)
{
	if (stage == INIT_BASE) {
		BaseConnectionManager* cc = FindModule<BaseConnectionManager*>::findGlobalModule();
		if (!cc) error("Could not find BaseConnectionManager module");

		cModule* parent = getParentModule();
		if (!parent) error("Parent module not found");

		const std::string moduleTypes = par("moduleType").stdstringValue();
		const std::string moduleNames = par("moduleName").stdstringValue();
		const std::string moduleDisplayStrings = par("moduleDisplayString").stdstringValue();
		moduleMapper.parseConfig(moduleTypes, moduleNames, moduleDisplayStrings);

		nodeManager = new TraCIVeinsNodeManager(&moduleMapper, parent, cc);
		TraCIScenarioManagerBase::setNodeManager(nodeManager);
	} else if (stage == INIT_LISTENER) {
		BaseWorldUtility* world = FindModule<BaseWorldUtility*>::findGlobalModule();
		if (world == NULL) error("Could not find BaseWorldUtility module");
		worldListener = new TraCIWorldListener(this, world);
		TraCIScenarioManagerBase::addListener(worldListener);

		obstacleListener = new TraCIObstacleListener(this);
		TraCIScenarioManagerBase::addListener(obstacleListener);
	}

	TraCIScenarioManagerBase::initialize(stage);
}

} // namespace Veins
