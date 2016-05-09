#ifndef VEINS_WORLD_TRACI_TRACIVEINSNODEMANAGER_H
#define VEINS_WORLD_TRACI_TRACIVEINSNODEMANAGER_H

#include "veins/modules/mobility/traci/TraCINodeManager.h"
#include <map>

class BaseConnectionManager;

namespace Veins {

class TraCIModuleMapper;

class TraCIVeinsNodeManager : public TraCINodeManager
{
	public:
		TraCIVeinsNodeManager(const TraCIModuleMapper*, cModule* parent, BaseConnectionManager*);

		void add(const std::string& id, const NodeData&, const std::string& vtype, simtime_t start);
		void remove(const std::string& id);
		void update(const std::string& id, const NodeData&);
		cModule* get(const std::string& id);
		void finish();
		size_t size();

	private:
		virtual void preInitialize(cModule*, const std::string& nodeId, const NodeData&);
		virtual void postInitialize(cModule*, const std::string& nodeId, const NodeData&);

		BaseConnectionManager* connections;
		const TraCIModuleMapper* moduleMapper;
		cModule* moduleParent;
		unsigned moduleIndex;
		std::map<std::string, cModule*> nodes;
};

} // namespace Veins

#endif
