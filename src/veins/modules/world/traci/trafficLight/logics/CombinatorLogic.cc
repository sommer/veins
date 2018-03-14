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

#include <algorithm>
#include <iomanip>
#include "veins/modules/world/traci/trafficLight/logics/CombinatorLogic.h"
#include "veins/modules/world/traci/trafficLight/util/ConflictGraph.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"


using Veins::TraCITrafficLightCombinatorLogic;
using Veins::ConflictGraph;
using PhaseTime = TraCITrafficLightCombinatorLogic::PhaseTime;
using PhaseSignal = TraCITrafficLightCombinatorLogic::PhaseSignal;
using ColorState = TraCITrafficLightCombinatorLogic::ColorState;

Define_Module(Veins::TraCITrafficLightCombinatorLogic);

/*
 * TODO: externalize and merge with implementation in OAF
 */
PhaseSignal makeYellowSignal(const PhaseSignal& lastSignal);


TraCITrafficLightCombinatorLogic::TraCITrafficLightCombinatorLogic():
    TraCITrafficLightAbstractLogic(),
    minGreenTime(0),
    maxGreenTime(0),
    stepGreenTime(1, SIMTIME_S),
    currentGreenTime(0),
    yellowTime(4, SIMTIME_S),
    redTime(2, SIMTIME_S)
{
}

TraCITrafficLightCombinatorLogic::~TraCITrafficLightCombinatorLogic()
{
}

void TraCITrafficLightCombinatorLogic::initialize()
{
    TraCITrafficLightAbstractLogic::initialize();
    // initialize own parameters
    minGreenTime = SimTime(par("minGreenTime").doubleValue());
    maxGreenTime = SimTime(par("maxGreenTime").doubleValue());
    stepGreenTime = SimTime(par("stepGreenTime").doubleValue());
    yellowTime = SimTime(par("yellowTime").doubleValue());
    redTime = SimTime(par("redTime").doubleValue());
    TraCITrafficLightInterface* iface = check_and_cast<TraCITrafficLightInterface*>(
        getParentModule()->getSubmodule("tlInterface")
    );
    // build conflict graph
    std::string xpath = "./junction[@id='" + iface->getExternalId() + "']";
    cXMLElement* junction = par("cgraph").xmlValue()->getElementByPath(xpath.c_str());
    ASSERT(junction != nullptr);
    ConflictGraph conflict_graph(junction);
    // initialize current phase
    lastPhase = make_pair(iface->getCurrentState(), 0.0);
    currentPhase = lastPhase;
    // initialize scheduler
    PhaseTime vehicleRecordTimeout = SimTime(par("vehicleRecordTimeout").doubleValue());
    double maxDistance = par("maxDistance").doubleValue();
    scheduler = CombinatorScheduler(conflict_graph, minGreenTime, maxGreenTime, stepGreenTime, vehicleRecordTimeout,
                                    maxDistance);
}

void TraCITrafficLightCombinatorLogic::finish()
{
}

void TraCITrafficLightCombinatorLogic::handleApplMsg(cMessage *msg)
{
    bool schedulerWasEmpty = scheduler.empty();
    if(std::string(msg->getName()) == "data") {
        // direct forwarded CAM
        CooperativeAwarenessMessage* cam = check_and_cast<CooperativeAwarenessMessage*>(msg);
        CAMRecord&& record = CAM2Record(cam);
        scheduler.handleVehicle(record);
    }
    if(schedulerWasEmpty && !scheduler.empty()) {
        // scheduler is no longer empty: re-enable
        cancelEvent(switchTimer);
        scheduleAt(simTime(), switchTimer);
    }
    delete msg;
}

void TraCITrafficLightCombinatorLogic::handleTlIfMsg(TraCITrafficLightMessage *tlMsg)
{
    delete tlMsg; // scheduling of switch events done in base class - done here for now
}

void TraCITrafficLightCombinatorLogic::handlePossibleSwitch()
{
    // clear timed out vehicles first
    scheduler.clearTimedOutVehicles();
    // distinguish between green, yellow and red phases
    switch(detectCurrentColorState()) {
        case ColorState::YELLOW:
            // dispatch red signal
            setNewState(PhaseSignal(lastPhase.first.size(), 'r'), redTime);
            break;

        case ColorState::RED:
            // dispatch new (green) phase
            setNewState(currentPhase.first, currentPhase.second);
            break;

        case ColorState::GREEN:
            if(scheduler.empty())
                return; // nothing to schedule
            // determine next green phase
            lastPhase = currentPhase;
            currentPhase = scheduler.makeNextPhase();
            // dispatch yellow signal
            setNewState(makeYellowSignal(lastPhase.first), yellowTime);
            break;
    }
}

void TraCITrafficLightCombinatorLogic::setNewState(const PhaseSignal& signal, const PhaseTime& duration)
{
    // send new signal state
    TraCITrafficLightMessage *pStateMsg = new TraCITrafficLightMessage("TrafficLightChangeMessage");
    pStateMsg->setChangedAttribute(TrafficLightAtrributeType::STATE);
    pStateMsg->setChangeSource(TrafficLightChangeSource::LOGIC);
    pStateMsg->setNewValue(signal.c_str());
    send(pStateMsg, "interface$o");
    // send new signal duration
    TraCITrafficLightMessage *pDurMsg = new TraCITrafficLightMessage("TrafficLightChangeMessage");
    pDurMsg->setChangedAttribute(TrafficLightAtrributeType::SWITCHTIME);
    pDurMsg->setChangeSource(TrafficLightChangeSource::LOGIC);
    pDurMsg->setNewValue(std::to_string((simTime() + duration).inUnit(SIMTIME_MS)).c_str());
    send(pDurMsg, "interface$o");
}

ColorState TraCITrafficLightCombinatorLogic::detectCurrentColorState() const
{
    // gain access to the current straffic light state via the interface
    TraCITrafficLightInterface* iface = check_and_cast<TraCITrafficLightInterface*>(
        getParentModule()->getSubmodule("tlInterface")
    );
    return getColorState(iface->getCurrentState());
}


ColorState TraCITrafficLightCombinatorLogic::getColorState(std::string signal_state) const
{
    if(signal_state.find("G") != signal_state.npos) // green phase
        return ColorState::GREEN;
    if (signal_state.find("y") != signal_state.npos) // yellow phase
        return ColorState::YELLOW;
    // red phase
    return ColorState::RED;
}

PhaseSignal makeYellowSignal(const PhaseSignal& lastSignal)
{
    PhaseSignal yellowSignal(lastSignal);
    std::replace(yellowSignal.begin(), yellowSignal.end(), 'G','y');
    std::replace(yellowSignal.begin(), yellowSignal.end(), 'g','y');
    return yellowSignal;
}
