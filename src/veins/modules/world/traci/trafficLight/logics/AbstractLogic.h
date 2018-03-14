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

#ifndef TRACITRAFFICLIGHTABSTRACTLOGIC_H_
#define TRACITRAFFICLIGHTABSTRACTLOGIC_H_

#include <omnetpp.h>
#include "veins/modules/messages/TraCITrafficLightMessage_m.h"

namespace Veins {
using omnetpp::cMessage;
using omnetpp::cSimpleModule;

/**
 * Base class to simplify implementation of traffic light logics
 *
 * already provides multiplexing of different message types to message handlers and a
 * special handler to be executed right before the TraCI server performs a phase switch
 */
class TraCITrafficLightAbstractLogic: public cSimpleModule {
public:
    TraCITrafficLightAbstractLogic();
    virtual ~TraCITrafficLightAbstractLogic();

protected:
    cMessage* switchTimer;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void handleSelfMsg(cMessage *msg);
    virtual void handleApplMsg(cMessage *msg) = 0;
    virtual void handleTlIfMsg(TraCITrafficLightMessage *tlMsg) = 0;
    virtual void handlePossibleSwitch() = 0;
};


} // namespace

#endif /* TRACITRAFFICLIGHTABSTRACTLOGIC_H_ */
