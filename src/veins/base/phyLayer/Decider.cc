#include "veins/base/phyLayer/Decider.h"

using namespace Veins;
using Veins::AirFrame;

bool DeciderResult::isSignalCorrect() const
{
    return isCorrect;
}

Decider::Decider(DeciderToPhyInterface* phy)
    : phy(phy)
    , notAgain(-1)
{
}

simtime_t Decider::processSignal(AirFrame* s)
{

    return -1;
}
