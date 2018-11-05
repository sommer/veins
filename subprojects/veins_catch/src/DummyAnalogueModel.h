#ifndef DUMMYANALOGUEMODEL_H_
#define DUMMYANALOGUEMODEL_H_

#include <cstdlib>

#include "../../../src/veins/base/phyLayer/AnalogueModel.h"

class DummyAnalogueModel : public AnalogueModel {
protected:
    const double factor;

public:
    DummyAnalogueModel(double factor)
        : factor(factor)
    {
    }

    virtual void filterSignal(AirFrame* frame, const Coord& sendersPos, const Coord& receiverPos);
    virtual void filterSignal(Signal* signal, const Coord& sendersPos, const Coord& receiverPos);
};

#endif /*DUMMYANALOGUEMODEL_H_*/
