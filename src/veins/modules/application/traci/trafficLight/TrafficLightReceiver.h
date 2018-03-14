//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __VEINS_TRAFFICLIGHTRECEIVER_H_
#define __VEINS_TRAFFICLIGHTRECEIVER_H_

#include <omnetpp.h>
#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"

using Veins::TraCIMobility;
using Veins::TraCICommandInterface;

class TrafficLightReceiver: public BaseWaveApplLayer {
protected:
    /** @brief this function is called upon receiving a BasicSafetyMessage, also referred to as a beacon  */
    virtual void onBSM(BasicSafetyMessage* bsm);

    virtual void onWSM(WaveShortMessage* wsm);
    virtual void onWSA(WaveServiceAdvertisment* wsa);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
