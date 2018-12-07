#include "veins/modules/analogueModel/PERModel.h"

#include "veins/base/messages/AirFrame_m.h"

using namespace Veins;
using Veins::AirFrame;

void PERModel::filterSignal(Signal* signal, const AntennaPosition& senderPos_, const AntennaPosition& receiverPos_)
{
    auto senderPos = senderPos_.getPositionAt();
    auto receiverPos = receiverPos_.getPositionAt();

    double attenuationFactor = 1; // no attenuation
    if (packetErrorRate > 0 && RNGCONTEXT uniform(0, 1) < packetErrorRate) {
        attenuationFactor = 0; // absorb all energy so that the receveir cannot receive anything
    }

    *signal *= attenuationFactor;
}
