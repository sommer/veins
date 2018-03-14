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

#ifndef SRC_VEINS_MODULES_WORLD_TRACI_TRAFFICLIGHT_LOGICS_TRACITRAFFICLIGHTEXAMPLELOGIC_H_
#define SRC_VEINS_MODULES_WORLD_TRACI_TRAFFICLIGHT_LOGICS_TRACITRAFFICLIGHTEXAMPLELOGIC_H_

#include "veins/base/utils/FindModule.h"
#include "veins/modules/messages/TraCITrafficLightMessage_m.h"
#include "veins/modules/world/traci/trafficLight/logics/AbstractLogic.h"

namespace Veins {

class TraCITrafficLightExampleLogic: public TraCITrafficLightAbstractLogic {
public:
    enum class ColorState {
        GREEN, YELLOW, RED
    };
    TraCITrafficLightExampleLogic();
    virtual ~TraCITrafficLightExampleLogic();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleApplMsg(cMessage *msg);
    virtual void handleTlIfMsg(TraCITrafficLightMessage *tlMsg);

    virtual void handlePossibleSwitch();
    virtual void setNewState(/*const signalScheme& signal,
     const phasetime_t& greenTime*/);
    //virtual signalScheme makeYellowScheme(const signalScheme& lastScheme) const;
    virtual bool isSuperposition(/*const signalScheme& major,
     const signalScheme& minor*/) const;

    /* void logPlatoonConfig(const std::string& colorWord,
     const signalScheme& signal, const phasetime_t& greenTime);*/
    void highlightScheduledConfig();

};

class TraCITrafficLightExampleLogicAccess {
public:
    TraCITrafficLightExampleLogic* get(cModule* host)
    {
        TraCITrafficLightExampleLogic* traci = FindModule<TraCITrafficLightExampleLogic*>::findSubModule(host);
        ASSERT(traci);
        return traci;
    };
};
}
#endif /* SRC_VEINS_MODULES_WORLD_TRACI_TRAFFICLIGHT_LOGICS_TRACITRAFFICLIGHTEXAMPLELOGIC_H_ */
