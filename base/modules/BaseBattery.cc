#include "BaseBattery.h"

#include "BaseUtility.h"

/**
 * Subscription to Blackboard should be in stage==0, and firing
 * notifications in stage==1 or later.
 *
 * NOTE: You have to call this in the initialize() function of the
 * inherited class!
 */
void BaseBattery::initialize(int stage) {
    if (stage == 0) {
        utility = FindModule<BaseUtility*>::findSubModule(findHost());
    }
}

cModule *BaseBattery::findHost(void)
{
	cModule *parent = getParentModule();
	cModule *node = this;

	// all nodes should be a sub module of the simulation which has no parent module!!!
	while( parent->getParentModule() != NULL ){
	node = parent;
	parent = node->getParentModule();
	}

	return node;
}
