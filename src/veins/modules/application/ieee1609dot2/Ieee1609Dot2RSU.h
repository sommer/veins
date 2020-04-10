/*
 * Ieee1609Dot2RSu.h
 *
 *  Created on: Apr 10, 2020
 *      Author: giuseppe
 */

#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

namespace veins {

/**
 * Small RSU Demo using IEEE 1609.2
 */
class VEINS_API Ieee1609Dot2RSU : public DemoBaseApplLayer {
protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;
};

} // namespace veins

