

#pragma once

#include <omnetpp.h>

#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2Message_m.h"


namespace veins {


class VEINS_API Ieee1609Dot2{
public:
    Ieee1609Dot2() { }
    Ieee1609Dot2Data* createSPDU(int type, const char * content);
    const char * processSPDU(Ieee1609Dot2Message* spdu);
};
}
