#ifndef VEINS_WORLD_TRACI_TRACISCENARIOMANAGER_H
#define VEINS_WORLD_TRACI_TRACISCENARIOMANAGER_H

#include "veins/modules/mobility/traci/TraCIScenarioManagerBase.h"

namespace Veins {

class TraCIScenarioManager : public TraCIScenarioManagerBase
{
};

class TraCIScenarioManagerAccess
{
	public:
		TraCIScenarioManager* get() {
			return FindModule<TraCIScenarioManager*>::findGlobalModule();
		};
};

} // namespace Veins

#endif
