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

#include "veins/modules/world/traci/trafficLight/logics/SimpleLogic.h"

using Veins::TraCITrafficLightSimpleLogic;

Define_Module(Veins::TraCITrafficLightSimpleLogic);

void TraCITrafficLightSimpleLogic::handleApplMsg(cMessage *msg) {
    TraCITrafficLightMessage *pStateMsg = new TraCITrafficLightMessage("TrafficLightChangeMessage");
    pStateMsg->setChangedAttribute(TrafficLightAtrributeType::STATE);
    pStateMsg->setChangeSource(TrafficLightChangeSource::LOGIC);
    pStateMsg->setNewValue("ggg");
    send(pStateMsg, "interface$o");
    delete msg; // just drop it
}

void TraCITrafficLightSimpleLogic::handleTlIfMsg(TraCITrafficLightMessage *tlMsg) {
    delete tlMsg; // just drop it
}

void TraCITrafficLightSimpleLogic::handlePossibleSwitch() {
    // do nothing - just let it happen
}
