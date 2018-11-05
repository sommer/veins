#include "DummyAnalogueModel.h"

//#include "../abstraction/AirFrame.h"

void DummyAnalogueModel::filterSignal(AirFrame *frame, const Coord& sendersPos, const Coord& receiverPos)
{
	Signal& signal = frame->getSignal();

    signal.addUniformAttenuation(factor);
}

void DummyAnalogueModel::filterSignal(Signal *signal, const Coord& sendersPos, const Coord& receiverPos)
{
    signal->addUniformAttenuation(factor);
}
