#ifndef VEINS_MOBILITY_TRACI_TIME_H
#define VEINS_MOBILITY_TRACI_TIME_H

#include <omnetpp.h>

namespace Veins {

struct TraCITime
{
	static const uint32_t min() {
		return 0;
	}

	static const uint32_t max() {
		return 0x7FFFFFFF;
	}

	static uint32_t from(simtime_t time) {
		return time.inUnit(SIMTIME_MS);
	}
};

} // namespace Veins

#endif
