#include "UnitDisk.h"
#include "FindModule.h"

Define_Module(UnitDisk);

NodeList * UnitDisk::canHear(BasePhyLayer* phy)
{
	NodeList *ret = new NodeList();
	const Coord *loc = FindModule<BaseUtility*>::findSubModule(phy->getNode())->getPos();
	for (NodeList::iterator i = nodes->begin();i!=nodes->end();i++)
	{
		if (*i == phy)
			continue;
		const Coord *loc2 = FindModule<BaseUtility*>::findSubModule((*i)->getNode())->getPos();
		//EV << "Distance from "<<phy->getNode()->index()<<" to "<<(*i)->getNode()->index()<<" is "<<loc2->distance(loc)<<endl;
		if (loc2->distance(loc)<=radioRange)
			ret->push_back(*i);
	}
	return ret;
}

void UnitDisk::initialize(int stage)
{
	CollisionsModel::initialize(stage);
	if (stage == 0)
		radioRange = par("radioRange");
}

