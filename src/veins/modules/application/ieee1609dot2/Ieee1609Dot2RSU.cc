/*
 * Ieee1609Dot2RSu.cc
 *
 *  Created on: Apr 10, 2020
 *      Author: giuseppe
 */

#include <veins/modules/application/ieee1609dot2/Ieee1609Dot2RSU.h>

#include "veins/modules/application/ieee1609dot2/Ieee1609Dot2Message_m.h"

using namespace veins;

Define_Module(veins::Ieee1609Dot2RSU);

void Ieee1609Dot2RSU::onWSA(DemoServiceAdvertisment* wsa)
{
    // if this RSU receives a WSA for service 42, it will tune to the chan
    if (wsa->getPsid() == 42) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
    }
}

void Ieee1609Dot2RSU::onWSM(BaseFrame1609_4* frame)
{
    Ieee1609Dot2Message* wsm = check_and_cast<Ieee1609Dot2Message*>(frame);

    // this rsu repeats the received traffic update in 2 seconds plus some random delay
    sendDelayedDown(wsm->dup(), 2 + uniform(0.01, 0.2));
}
