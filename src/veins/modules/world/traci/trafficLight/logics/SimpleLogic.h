//
// Copyright (C) 2015 Dominik Buse <dbuse@mail.uni-paderborn.de>
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

#ifndef TRACITRAFFICLIGHTSIMPLELOGIC_H_
#define TRACITRAFFICLIGHTSIMPLELOGIC_H_


#include "veins/base/utils/FindModule.h"
#include "veins/modules/world/traci/trafficLight/logics/AbstractLogic.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"


namespace Veins {
class TraCITrafficLightSimpleLogic: public TraCITrafficLightAbstractLogic {

public:
    using signalScheme = std::string;
protected:
    virtual void handleApplMsg(cMessage *msg);
    virtual void handleTlIfMsg(TraCITrafficLightMessage *tlMsg);
    virtual void handlePossibleSwitch();
};

class TraCITrafficLightSimpleLogicAccess {
public:
    TraCITrafficLightSimpleLogic* get(cModule* host)
    {
        TraCITrafficLightSimpleLogic* traci = FindModule<TraCITrafficLightSimpleLogic*>::findSubModule(host);
        ASSERT(traci);
        return traci;
    };
};

} // namespace

#endif /* TRACITRAFFICLIGHTSIMPLELOGIC_H_ */
