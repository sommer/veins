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
#include "veins/modules/world/traci/trafficLight/logics/MonctonLogic.h"
#include "veins/modules/world/traci/trafficLight/util/ConflictGraph.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"


using Veins::TraCITrafficLightMonctonLogic;
using Veins::ConflictGraph;

Define_Module(Veins::TraCITrafficLightMonctonLogic);

TraCITrafficLightMonctonLogic::TraCITrafficLightMonctonLogic():
    TraCITrafficLightAbstractLogic(),
    minGreenTime(0),
    maxGreenTime(0),
    stepGreenTime(1, SIMTIME_S),
    currentGreenTime(0),
    yellowTime(4, SIMTIME_S),
    redTime(2, SIMTIME_S)
{
}

TraCITrafficLightMonctonLogic::~TraCITrafficLightMonctonLogic()
{
}

void TraCITrafficLightMonctonLogic::initialize()
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
    // build collection of possible scenarios
    MonctonScheduler::ScenarioSet possibleScenarios;
    auto logic = iface->getCurrentLogic();
    auto scenario_nr = 0;
    for(auto&& itPhase : logic.phases) {
        if(getColorState(itPhase.state) == ColorState::GREEN) {
            MonctonScheduler::Scenario scenario_approaches;
            auto signal_nr = 0;
            for(auto&& itSignal : itPhase.state) {
                if(itSignal == 'G') {
                    scenario_approaches.push_back(signal_nr);
                }
                ++signal_nr;
            }
            possibleScenarios[scenario_nr] = scenario_approaches;
            ++scenario_nr;
        }
    }
    // initialize scheduler
    PhaseTime vehicleRecordTimeout = SimTime(par("vehicleRecordTimeout").doubleValue());
    scheduler = MonctonScheduler(conflict_graph, possibleScenarios, vehicleRecordTimeout);
}

void TraCITrafficLightMonctonLogic::finish()
{
}

void TraCITrafficLightMonctonLogic::handleApplMsg(cMessage *msg)
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

void TraCITrafficLightMonctonLogic::handleTlIfMsg(TraCITrafficLightMessage *tlMsg)
{
    delete tlMsg; // scheduling of switch events done in base class - done here for now
}

void TraCITrafficLightMonctonLogic::handlePossibleSwitch()
{
    // clear timed out vehicles first
    scheduler.clearTimedOutVehicles();
    TraCITrafficLightInterface* iface = check_and_cast<TraCITrafficLightInterface*>(
        getParentModule()->getSubmodule("tlInterface")
    );

    // distinguish between green, yellow and red phases
    switch(detectCurrentColorState()) {
        case ColorState::YELLOW:
            break; // nothting to do during yellow

        case ColorState::RED:
            // determine next green phase
            // TODO: replace with change switch_time message
            {
                unsigned int nextPhase = scheduler.scheduleNextPhaseNr();
                currentGreenTime = minGreenTime;
//                std::cout << iface->getExternalId() << ": Set Phase Nr to " << nextPhase << " for minDur=" << minGreenTime << "\n";
                iface->setCurrentPhaseByNr(nextPhase, true);
                iface->setRemainingDuration(minGreenTime, true);
            }
            break;

        case ColorState::GREEN:
            // prolong or end green phase
            if(currentGreenTime < maxGreenTime && scheduler.isPhaseToBeProlonged()) {
                // TODO: replace with change switch_time message
                iface->setRemainingDuration(stepGreenTime);
                currentGreenTime += stepGreenTime;
//                std::cout << iface->getExternalId() << ": Prolong green phase, now at " << currentGreenTime << "s\n";
            }
            break;
    }
}

TraCITrafficLightMonctonLogic::ColorState TraCITrafficLightMonctonLogic::detectCurrentColorState() const
{
    // gain access to the current straffic light state via the interface
    TraCITrafficLightInterface* iface = check_and_cast<TraCITrafficLightInterface*>(
        getParentModule()->getSubmodule("tlInterface")
    );
    return getColorState(iface->getCurrentState());
}


TraCITrafficLightMonctonLogic::ColorState TraCITrafficLightMonctonLogic::getColorState(std::string signal_state) const
{
    if(signal_state.find("G") != signal_state.npos) // green phase
        return ColorState::GREEN;
    if (signal_state.find("y") != signal_state.npos) // yellow phase
        return ColorState::YELLOW;
    // red phase
    return ColorState::RED;
}
