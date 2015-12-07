#ifndef VEINS_WORLD_TRACI_TRACISCENARIOMANAGER_H
#define VEINS_WORLD_TRACI_TRACISCENARIOMANAGER_H

#include "veins/modules/mobility/traci/TraCIScenarioManagerBase.h"

namespace Veins {

// forward declarations
class TraCIWorldListener;

class TraCIScenarioManager : public TraCIScenarioManagerBase
{
        public:
		TraCIScenarioManager();
		~TraCIScenarioManager();
		void initialize(int stage);

        private:
		TraCIWorldListener* worldListener;
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
