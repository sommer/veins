#ifndef VEINS_WORLD_TRACI_TRACIWORLDLISTENER_H
#define VEINS_WORLD_TRACI_TRACIWORLDLISTENER_H

#include "veins/modules/mobility/traci/TraCIListener.h"

class BaseWorldUtility;

namespace Veins {

class TraCIScenarioManagerBase;

class TraCIWorldListener : public TraCIListener
{
	public:
		TraCIWorldListener(TraCIScenarioManagerBase*, BaseWorldUtility*);
		virtual void init();

	private:
		TraCIScenarioManagerBase* manager;
		BaseWorldUtility* world;
};

} // namespace Veins

#endif // VEINS_WORLD_TRACI_TRACIWORLDLISTENER_H

