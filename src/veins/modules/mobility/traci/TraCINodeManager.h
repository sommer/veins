#ifndef VEINS_WORLD_TRACI_TRACINODEMANAGER_H
#define VEINS_WORLD_TRACI_TRACINODEMANAGER_H

#include "veins/base/utils/Coord.h"
#include "veins/modules/mobility/traci/TraCIVehicleSignals.h"
#include <string>
#include <omnetpp.h>

namespace Veins {

class TraCINodeManager
{
	public:
		struct NodeData
		{
			NodeData() : speed(-1), angle(-1) {}
			Coord position;
			std::string road_id;
			double speed;
			double angle;
			TraCIVehicleSignal signals;
		};

		virtual void add(const std::string& nodeId, const NodeData&, const std::string& vehicleType, simtime_t) = 0;
		virtual void remove(const std::string& nodeId) = 0;
		virtual void update(const std::string& nodeId, const NodeData&) = 0;
		virtual cModule* get(const std::string& nodeId) = 0;
		virtual void finish() = 0;
		virtual size_t size() = 0;
		virtual ~TraCINodeManager() {}
};

} // namespace Veins

#endif
