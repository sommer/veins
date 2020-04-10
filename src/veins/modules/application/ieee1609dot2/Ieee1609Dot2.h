

#pragma once

#include <omnetpp.h>

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2Message_m.h"


namespace veins {


class VEINS_API Ieee1609Dot2 : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;

    Ieee1609Dot2Message* createSPDU(int type, const char * content);
    void processSPDU(Ieee1609Dot2Message* spdu);

protected:
    simtime_t lastDroveAt;
    bool sentMessage;
    int currentSubscribedServiceId;

protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
};

}
