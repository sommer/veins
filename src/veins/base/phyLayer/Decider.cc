#include "veins/base/phyLayer/Decider.h"

using namespace Veins;

bool DeciderResult::isSignalCorrect() const
{
    return isCorrect;
}

Decider::Decider(cComponent* owner, DeciderToPhyInterface* phy)
    : HasLogProxy(owner)
    , phy(phy)
    , notAgain(-1)
{
}

simtime_t Decider::processSignal(AirFrame* s)
{

    return -1;
}
