#include "BaseUtility.h"
#include "BaseWorldUtility.h"
#include "FindModule.h"
#include <assert.h>
#include "Move.h"

Define_Module(BaseUtility);

#ifndef coreEV
#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev <<getParentModule()->getName()<<"["<<getParentModule()->getIndex()<<"]::BaseUtility: "
#endif

void BaseUtility::initialize(int stage) {
	Blackboard::initialize(stage);

	if (stage == 0) {
        // subscribe to position changes
        Move moveBBItem;
        catMove = subscribe(this, &moveBBItem, findHost()->getId());

        catHostState = subscribe(this, &hostState, findHost()->getId());
        hostState.set(HostState::ACTIVE);
	}
}

cModule *BaseUtility::findHost(void)
{
	return FindModule<>::findHost(this);
}

/**
 * BaseUtility subscribes itself to position and host state changes to
 * synchronize the hostPosition and -state.
 */
void BaseUtility::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    //BaseModule::receiveBBItem(category, details, scopeModuleId);

    if(category == catMove)
    {
        const Move* m = static_cast<const Move*>(details);
        pos = m->getStartPos();
        coreEV << "new HostMove: " << m->info() << endl;
    }
    else if(category == catHostState)
    {
    	const HostState* state = static_cast<const HostState*>(details);
		hostState = *state;
		coreEV << "new HostState: " << hostState.info() << endl;
    }
}
