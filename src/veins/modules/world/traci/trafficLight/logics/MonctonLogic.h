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

#ifndef TRACITRAFFICLIGHTMONCTONLOGIC_H_
#define TRACITRAFFICLIGHTMONCTONLOGIC_H_

#include <iostream>
#include <fstream>
#include "veins/base/utils/FindModule.h"
#include "veins/modules/world/traci/trafficLight/util/MonctonScheduler.h"
#include "veins/modules/world/traci/trafficLight/logics/AbstractLogic.h"

namespace Veins {
class TraCITrafficLightMonctonLogic: public TraCITrafficLightAbstractLogic {
public:
    enum class ColorState {
        GREEN, YELLOW, RED
    };

    using SignalScheme = MonctonScheduler::SignalScheme;
    using PhaseTime = MonctonScheduler::PhaseTime;

    TraCITrafficLightMonctonLogic();
    virtual ~TraCITrafficLightMonctonLogic();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleApplMsg(cMessage *msg);
    virtual void handleTlIfMsg(TraCITrafficLightMessage *tlMsg);
    virtual void handlePossibleSwitch();

    MonctonScheduler scheduler;
private:
    /*
     * minimum green time for each phase in the cyle
     */
    PhaseTime minGreenTime;
    /*
     * maximum green time for each phase in the cyle
     */
    PhaseTime maxGreenTime;
    /*
     * lapse of time by which green phases are extended
     */
    PhaseTime stepGreenTime;
    /*
     * duration the current green time has already been shown
     */
    PhaseTime currentGreenTime;
    /*
     * time for yellow phases
     */
    PhaseTime yellowTime;
    /*
     * time for red phases
     */
    PhaseTime redTime;

    ColorState detectCurrentColorState() const;
    ColorState getColorState(std::string signal_state) const;
};

class TraCITrafficLightMonctonLogicAccess {
public:
    TraCITrafficLightMonctonLogic* get(cModule* host)
    {
        TraCITrafficLightMonctonLogic* traci = FindModule<TraCITrafficLightMonctonLogic*>::findSubModule(host);
        ASSERT(traci);
        return traci;
    };
};

} // namespace

#endif /* TRACITRAFFICLIGHTMONCTONLOGIC_H_ */
