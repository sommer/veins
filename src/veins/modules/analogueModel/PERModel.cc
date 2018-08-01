#include "veins/modules/analogueModel/PERModel.h"

#include "veins/base/messages/AirFrame_m.h"

using namespace Veins;
using Veins::AirFrame;

void PERModel::filterSignal(Signal* signal, const Coord& sendersPos, const Coord& receiverPos)
{

    double attenuationFactor = 1; // no attenuation
    if (packetErrorRate > 0 && RNGCONTEXT uniform(0, 1) < packetErrorRate) {
        attenuationFactor = 0; // absorb all energy so that the receveir cannot receive anything
    }

    signal->addUniformAttenuation(attenuationFactor);
}
