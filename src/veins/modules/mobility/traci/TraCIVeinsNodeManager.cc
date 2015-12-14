#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCIModuleMapper.h"
#include "veins/modules/mobility/traci/TraCIVeinsNodeManager.h"
#include "veins/modules/mobility/traci/TraCIScenarioManagerInet.h"

namespace Veins {

TraCIVeinsNodeManager::TraCIVeinsNodeManager(const TraCIModuleMapper* mapper, cModule* parent, BaseConnectionManager* cc) :
    connections(cc), moduleMapper(mapper), moduleParent(parent), moduleIndex(0)
{
}

void TraCIVeinsNodeManager::add(const std::string& nodeId, const NodeData& data, const std::string& vehicleType, simtime_t start)
{
	TraCIModuleMapper::ModuleConfig moduleConfig = moduleMapper->getModuleConfig(vehicleType);
	if (moduleConfig.type == "") return;

	cModuleType* moduleType = cModuleType::get(moduleConfig.type.c_str());
	if (!moduleType)
		opp_error("module type \"%\" not found", moduleConfig.type.c_str());

	cModule* node = moduleType->create(moduleConfig.name.c_str(), moduleParent, moduleIndex, moduleIndex);
	node->finalizeParameters();
	node->setDisplayString(moduleConfig.displayString.c_str());
	node->buildInside();
	node->scheduleStart(start);
	++moduleIndex;

	preInitialize(node, nodeId, data);
	node->callInitialize();
	postInitialize(node, nodeId, data);
	nodes[nodeId] = node;
}

void TraCIVeinsNodeManager::remove(const std::string& nodeId)
{
	std::map<std::string, cModule*>::iterator found = nodes.find(nodeId);
	if (found != nodes.end()) {
		cModule* node = found->second;
		connections->unregisterNic(node->getSubmodule("nic"));
		nodes.erase(found);
		node->callFinish();
		node->deleteModule();
	} else {
		opp_error("no vehicle with id \"%s\" found", nodeId.c_str());
	}
}

void TraCIVeinsNodeManager::update(const std::string& nodeId, const NodeData& data)
{
	cModule* mod = nodes.at(nodeId);
	for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
		cModule* submod = SUBMODULE_ITERATOR_TO_MODULE(iter);
		ifInetTraCIMobilityCallNextPosition(submod, data.position, data.road_id, data.speed, data.angle);
		TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
		if (mm) {
			EV << "module " << nodeId << " moving to " << data.position.x << "," << data.position.y << endl;
			mm->nextPosition(data.position, data.road_id, data.speed, data.angle, data.signals);
		}
	}
}

cModule* TraCIVeinsNodeManager::get(const std::string& nodeId)
{
	cModule* node = 0;
	std::map<std::string, cModule*>::iterator found = nodes.find(nodeId);
	if (found != nodes.end()) {
		node = found->second;
	}
	return node;
}

void TraCIVeinsNodeManager::finish()
{
	while (nodes.begin() != nodes.end()) {
		remove(nodes.begin()->first);
	}
}

size_t TraCIVeinsNodeManager::size()
{
	return nodes.size();
}

void TraCIVeinsNodeManager::preInitialize(cModule* node, const std::string& nodeId, const NodeData& data)
{
	for (cModule::SubmoduleIterator iter(node); !iter.end(); iter++) {
		cModule* submod = SUBMODULE_ITERATOR_TO_MODULE(iter);
		ifInetTraCIMobilityCallPreInitialize(submod, nodeId, data.position, data.road_id, data.speed, data.angle);
		TraCIMobility* mob = dynamic_cast<TraCIMobility*>(submod);
		if (mob) {
			mob->preInitialize(nodeId, data.position, data.road_id, data.speed, data.angle);
		}
	}
}

void TraCIVeinsNodeManager::postInitialize(cModule* node, const std::string& nodeId, const NodeData& data)
{
	for (cModule::SubmoduleIterator iter(node); !iter.end(); iter++) {
		cModule* submod = SUBMODULE_ITERATOR_TO_MODULE(iter);
		TraCIMobility* mob = dynamic_cast<TraCIMobility*>(submod);
		if (mob) {
			mob->changePosition();
		}
	}
}

} // namespace Veins
