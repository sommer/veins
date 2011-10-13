#include "BaseUtility.h"
#include "BaseWorldUtility.h"
#include "FindModule.h"
#include <assert.h>
#include "Move.h"
#include "BaseMobility.h"

Define_Module(BaseUtility);

void BaseUtility::initialize(int stage) {
	Blackboard::initialize(stage);

	if (stage == 0) {
        // subscribe to position changes
        Move moveBBItem;
        catMove = subscribe(this, &moveBBItem, findHost()->getId());

        catHostState = subscribe(this, &hostState, findHost()->getId());
        hostState.set(HostState::ACTIVE);
	}
	else if(stage == 1) {
		cModule* host = findHost();
		//check if necessary host modules are available
		//mobility module
		if(!FindModule<BaseMobility*>::findSubModule(host)) {
			opp_warning("No mobility module found in host with index %d!", host->getIndex());
		}
	}
}

cModule *BaseUtility::findHost(void)
{
	return FindModule<>::findHost(this);
}

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

std::string BaseUtility::logName(void)
{
    std::ostringstream ost;
	cModule *parent = findHost();
	parent->hasPar("logName") ?
		ost << parent->par("logName").stringValue() : ost << parent->getName();
	ost << "[" << parent->getIndex() << "]";
	return ost.str();
}
