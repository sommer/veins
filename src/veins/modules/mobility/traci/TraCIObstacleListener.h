#ifndef VEINS_WORLD_TRACI_TRACIOBSTACLELISTENER_H
#define VEINS_WORLD_TRACI_TRACIOBSTACLELISTENER_H

#include "veins/modules/mobility/traci/TraCIListener.h"

namespace Veins {

class TraCIScenarioManagerBase;

class TraCIObstacleListener : public TraCIListener
{
	public:
		TraCIObstacleListener(TraCIScenarioManagerBase*);
		virtual void init();

	private:
		TraCIScenarioManagerBase* manager;
};

} // namespace Veins

#endif // VEINS_WORLD_TRACI_TRACIOBSTACLELISTENER_H

