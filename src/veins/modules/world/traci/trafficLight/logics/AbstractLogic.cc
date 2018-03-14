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

#include "veins/modules/world/traci/trafficLight/logics/AbstractLogic.h"

using Veins::TraCITrafficLightAbstractLogic;
using namespace omnetpp;


TraCITrafficLightAbstractLogic::TraCITrafficLightAbstractLogic():
    cSimpleModule(),
    switchTimer(nullptr)
{
}

TraCITrafficLightAbstractLogic::~TraCITrafficLightAbstractLogic()
{
    cancelAndDelete(switchTimer);
}

void TraCITrafficLightAbstractLogic::initialize()
{
    switchTimer = new cMessage("trySwitch");
}

void TraCITrafficLightAbstractLogic::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
        handleSelfMsg(msg);
    } else if(msg->arrivedOn("interface$i")) {
        TraCITrafficLightMessage* tlMsg = check_and_cast<TraCITrafficLightMessage*>(msg);
        // always check for changed switch time and (re-)schedule switch handler if so
        if(tlMsg->getChangedAttribute() == TrafficLightAtrributeType::SWITCHTIME) {
            // schedule handler right before the switch
            cancelEvent(switchTimer);
            // make sure the message is not scheduled to the past
            simtime_t nextTick = std::max(SimTime(std::stoi(tlMsg->getNewValue()), SIMTIME_MS), simTime());
            scheduleAt(nextTick, switchTimer);
        }
        // defer further handling to subclass implementation
        handleTlIfMsg(tlMsg);
    } else if (msg->arrivedOn("applLayer$i")) {
        handleApplMsg(msg);
    } else {
        error("Unknown message arrived on %s", msg->getArrivalGate()->getName());
    }
}

void TraCITrafficLightAbstractLogic::handleSelfMsg(cMessage *msg)
{
    if(msg == switchTimer) {
        handlePossibleSwitch();
    }
}
