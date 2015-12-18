#include "veins/modules/analogueModel/PERModel.h"

#include "veins/base/messages/AirFrame_m.h"

using Veins::AirFrame;

void PERModel::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos) {
	Signal&   signal = frame->getSignal();
	//simtime_t start  = signal.getReceptionStart();
	//simtime_t end    = signal.getReceptionEnd();

	double attenuationFactor = 1;  // no attenuation
	if(packetErrorRate > 0 && RNGCONTEXT uniform(0, 1) < packetErrorRate) {
		attenuationFactor = 0;  // absorb all energy so that the receveir cannot receive anything
	}

	TimeMapping<Linear>* attMapping = new TimeMapping<Linear> ();
	Argument arg;
	attMapping->setValue(arg, attenuationFactor);
	signal.addAttenuation(attMapping);
}



