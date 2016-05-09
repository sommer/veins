#ifndef VEINS_WORLD_TRACI_TRACISCENARIOMANAGER_H
#define VEINS_WORLD_TRACI_TRACISCENARIOMANAGER_H

#include "veins/modules/mobility/traci/TraCIScenarioManagerBase.h"
#include "veins/modules/mobility/traci/TraCIModuleMapper.h"

namespace Veins {

// forward declarations
class TraCINodeManager;
class TraCIObstacleListener;
class TraCIWorldListener;

class TraCIScenarioManager : public TraCIScenarioManagerBase
{
        public:
		TraCIScenarioManager();
		~TraCIScenarioManager();
		void initialize(int stage);

        private:
		TraCIModuleMapper moduleMapper;
		TraCIWorldListener* worldListener;
		TraCIObstacleListener* obstacleListener;
		TraCINodeManager* nodeManager;
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
