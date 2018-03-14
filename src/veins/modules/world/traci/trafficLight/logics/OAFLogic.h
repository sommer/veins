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

#ifndef TRACITRAFFICLIGHTOAFLOGIC_H_
#define TRACITRAFFICLIGHTOAFLOGIC_H_

#include <iostream>
#include <fstream>
#include "veins/base/utils/FindModule.h"
#include "veins/modules/world/traci/trafficLight/util/OAFScheduler.h"
#include "veins/modules/world/traci/trafficLight/logics/AbstractLogic.h"

namespace Veins {
class TraCITrafficLightOAFLogic: public TraCITrafficLightAbstractLogic {
public:
    enum class ColorState {
        GREEN, YELLOW, RED
    };

    using signalScheme = OAFScheduler::signalScheme;
    using approachPlatoonConfig = OAFScheduler::approachPlatoonConfig;
    using phasetime_t = OAFScheduler::greenTime;

    TraCITrafficLightOAFLogic();
    virtual ~TraCITrafficLightOAFLogic();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleApplMsg(cMessage *msg);
    virtual void handleTlIfMsg(TraCITrafficLightMessage *tlMsg);
    virtual void handlePossibleSwitch();
    virtual void setNewState(const signalScheme& signal, const phasetime_t& greenTime);
    virtual signalScheme makeYellowScheme(const signalScheme& lastScheme) const;
    virtual bool isSuperposition(const signalScheme& major, const signalScheme& minor) const;

    void logPlatoonConfig(const std::string& colorWord, const signalScheme& signal, const phasetime_t& greenTime);
    void highlightScheduledConfig();

    OAFScheduler scheduler;
    std::ofstream customLog;
private:
    /**
     * maximum green time for the current phase
     *
     * called MAX-OUT by Pandit et al.
     */
    simtime_t maxGreenTime;
    /**
     * time for yellow phases
     */
    simtime_t yellowTime;
    /**
     * time for red phases
     */
    simtime_t redTime;
    /**
     * last green signal that was dispatched
     */
    signalScheme lastGreenSignal;
    /**
     * which color is currently displayed
     */
    ColorState colorState;
    /**
     * configuration scheduled to be dispatched next
     */
    approachPlatoonConfig scheduledConfig;
};

class TraCITrafficLightOAFLogicAccess {
public:
    TraCITrafficLightOAFLogic* get(cModule* host)
    {
        TraCITrafficLightOAFLogic* traci = FindModule<TraCITrafficLightOAFLogic*>::findSubModule(host);
        ASSERT(traci);
        return traci;
    };
};

} // namespace

#endif /* TRACITRAFFICLIGHTOAFLOGIC_H_ */
