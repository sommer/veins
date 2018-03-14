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

#include "TrafficLightReceiver.h"

using Veins::TraCIMobilityAccess;

Define_Module(TrafficLightReceiver);

void TrafficLightReceiver::initialize()
{
    EV << "Initialized" << std::endl;
}

void TrafficLightReceiver::handleMessage(cMessage *msg)
{

}
void TrafficLightReceiver::onBSM(BasicSafetyMessage* bsm) {
    if(std::string(bsm->getPsc()) == "CAM") {
        // directly forward to logic
        send(bsm->dup(), "logic$o");
    }
}

void TrafficLightReceiver::onWSA(WaveServiceAdvertisment* wsa) {
    //if this RSU receives a WSA for service 42, it will tune to the chan
    if (wsa->getPsid() == 42) {
        mac->changeServiceChannel(wsa->getTargetChannel());
    }
}

void TrafficLightReceiver::onWSM(WaveShortMessage* wsm) {
    //this rsu repeats the received traffic update in 2 seconds plus some random delay
    wsm->setSenderAddress(myId);
    sendDelayedDown(wsm->dup(), 2 + uniform(0.01,0.2));
}

