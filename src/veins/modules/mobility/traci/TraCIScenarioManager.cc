#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCIWorldListener.h"

Define_Module(Veins::TraCIScenarioManager);

namespace Veins {

TraCIScenarioManager::TraCIScenarioManager() :
	worldListener(0)
{
}

TraCIScenarioManager::~TraCIScenarioManager()
{
	delete(worldListener);
}

void TraCIScenarioManager::initialize(int stage)
{
	if (stage == INIT_LISTENER) {
		BaseWorldUtility* world = FindModule<BaseWorldUtility*>::findGlobalModule();
		if (world == NULL) error("Could not find BaseWorldUtility module");
		worldListener = new TraCIWorldListener(this, world);
		TraCIScenarioManagerBase::addListener(worldListener);
	}

	TraCIScenarioManagerBase::initialize(stage);
}

} // namespace Veins
