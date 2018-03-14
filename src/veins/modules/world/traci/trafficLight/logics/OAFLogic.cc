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
#include "veins/modules/world/traci/trafficLight/logics/OAFLogic.h"
#include "veins/modules/world/traci/trafficLight/util/ConflictGraph.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"


using Veins::TraCITrafficLightOAFLogic;
using Veins::ConflictGraph;

Define_Module(Veins::TraCITrafficLightOAFLogic);

TraCITrafficLightOAFLogic::TraCITrafficLightOAFLogic():
    TraCITrafficLightAbstractLogic(),
    maxGreenTime(0),
    yellowTime(4, SIMTIME_S),
    redTime(2, SIMTIME_S),
    lastGreenSignal(""),
    colorState(ColorState::GREEN),
    scheduledConfig()
{
}

TraCITrafficLightOAFLogic::~TraCITrafficLightOAFLogic()
{
}

void TraCITrafficLightOAFLogic::initialize()
{
    TraCITrafficLightAbstractLogic::initialize();
    // initialize own parameters
    maxGreenTime = SimTime(par("maxGreenTime").doubleValue());
    yellowTime = SimTime(par("yellowTime").doubleValue());
    redTime = SimTime(par("redTime").doubleValue());
    TraCITrafficLightInterface* iface = check_and_cast<TraCITrafficLightInterface*>
                                        (getParentModule()->getSubmodule("tlInterface"));
    // build conflict graph
    std::string xpath = "./junction[@id='" + iface->getExternalId() + "']";
    cXMLElement* junction = par("cgraph").xmlValue()->getElementByPath(xpath.c_str());
    ASSERT(junction != nullptr);
    ConflictGraph conflict_graph(junction);
    // initialize scheduler
    double approachingSpeed = par("approachingSpeed").doubleValue();
    double junctionProximityThreshold = par("junctionProximityThreshold").doubleValue();
    simtime_t vehicleRecordTimeout = SimTime(par("vehicleRecordTimeout").doubleValue());
    double scoreGainPerVehicle = par("scoreGainPerVehicle").doubleValue();
    double maxDistance = par("maxDistance").doubleValue();
    scheduler = OAFScheduler(conflict_graph, approachingSpeed, maxDistance, junctionProximityThreshold,
            vehicleRecordTimeout, scoreGainPerVehicle);
    // custom logging
    std::string customLogPath(par("customLogPath").stdstringValue());
    std::string logPath(customLogPath + iface->getExternalId() + ".log");
    customLog.open(logPath, std::ios::out | std::ios::trunc);
    customLog.setf(std::ios::fixed, std::ios::floatfield);
    customLog.precision(2);
}

void TraCITrafficLightOAFLogic::finish()
{
    if(customLog.is_open()) {
        customLog.close();
    }
}

void TraCITrafficLightOAFLogic::handleApplMsg(cMessage *msg)
{
    bool schedulerWasEmpty = scheduler.empty();
    if(std::string(msg->getName()) == "data") {
        // direct forwarded CAM
        CooperativeAwarenessMessage* cam = check_and_cast<CooperativeAwarenessMessage*>(msg);
        CAMRecord&& record = CAM2Record(cam);
        bool is_relevant = scheduler.handleVehicle(record);

        auto&& iface(TraCIScenarioManagerAccess().get()->getCommandInterface());
        // paint vehicle to show it has been noted
        auto vehcolor = iface->vehicle(record.vehicleId).getColor();
        if(is_relevant) {
            if(!(vehcolor.red == vehcolor.green && vehcolor.green == vehcolor.blue)) {
                iface->vehicle(record.vehicleId).setColor({192, 192, 192, 0});
            }
        } else {
            if(vehcolor.red == vehcolor.green && vehcolor.blue == vehcolor.green) {
                // vehicle is *no longer* relevant
                iface->vehicle(record.vehicleId).setColor({240, 0, 0, 0});
            }
        }
    }
    if(schedulerWasEmpty && !scheduler.empty()) {
        // scheduler is no longer empty: re-enable
        cancelEvent(switchTimer);
        scheduleAt(simTime(), switchTimer);
    }
    delete msg;
}

void TraCITrafficLightOAFLogic::handleTlIfMsg(TraCITrafficLightMessage *tlMsg)
{
    delete tlMsg; // scheduling of switch events done in base class - done here for now
}

void TraCITrafficLightOAFLogic::handlePossibleSwitch()
{
    // clean up known vehicles first
    scheduler.clearTimedOutVehicles();
    signalScheme signalToDispatch;
    phasetime_t signalDurationToDispatch;
    std::string colorWord;
    switch(colorState) {
    case ColorState::YELLOW: // dispatch red signal next
        signalToDispatch = signalScheme(lastGreenSignal.size(), 'r');
        signalDurationToDispatch = redTime;
        colorState = ColorState::RED;
        colorWord = "Red";
        break;

    case ColorState::RED: // dispatch green signal next
        signalToDispatch = scheduler.makeSignalScheme(scheduledConfig);
        signalDurationToDispatch = scheduler.estimateMaxGreenTime(scheduledConfig);
        lastGreenSignal = signalToDispatch;
        colorState = ColorState::GREEN;
        colorWord = "Green";
        logPlatoonConfig(colorWord, signalToDispatch, signalDurationToDispatch);
        break;

    case ColorState::GREEN: // schedule new configuration and...
        if(scheduler.empty()) {
            // ... do nothing and wait for next call triggered by handleApplMsg (no registered vehicles available)
            EV << "Currently no vehicles to schedule";
            return;
        } else  {
            scheduledConfig = scheduler.scheduleNextPlatoons(maxGreenTime);
            highlightScheduledConfig();
            if(lastGreenSignal.empty()
                    || isSuperposition(scheduler.makeSignalScheme(scheduledConfig), lastGreenSignal)) {
                // ...dispatch next green phase directly (previous signal empty (startup) or prolonged)
                colorState = ColorState::GREEN;
                colorWord = "Continued Green";
                signalToDispatch = scheduler.makeSignalScheme(scheduledConfig);
                signalDurationToDispatch = scheduler.estimateMaxGreenTime(scheduledConfig);
                lastGreenSignal = signalToDispatch;
                logPlatoonConfig(colorWord, signalToDispatch, signalDurationToDispatch);
            } else {
                // ... dispatch yellow phase (signal state changed)
                colorState = ColorState::YELLOW;
                colorWord = "Yellow";
                signalToDispatch = makeYellowScheme(lastGreenSignal);
                signalDurationToDispatch = yellowTime;
            }
        }
        break;
    }
    setNewState(signalToDispatch, signalDurationToDispatch);
}

void TraCITrafficLightOAFLogic::setNewState(const signalScheme& signal, const phasetime_t& greenTime)
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
    pDurMsg->setNewValue(std::to_string((simTime() + greenTime).inUnit(SIMTIME_MS)).c_str());
    send(pDurMsg, "interface$o");
}

TraCITrafficLightOAFLogic::signalScheme TraCITrafficLightOAFLogic::makeYellowScheme(
    const signalScheme& lastScheme) const
{
    signalScheme yellowScheme(lastScheme);
    std::replace(yellowScheme.begin(), yellowScheme.end(), 'G','y');
    std::replace(yellowScheme.begin(), yellowScheme.end(), 'g','y');
    return yellowScheme;
}

bool TraCITrafficLightOAFLogic::isSuperposition(const signalScheme& major, const signalScheme& minor) const
{
    for(size_t i = 0; i < major.size(); ++i) {
        if(std::tolower(minor.at(i)) == 'g' and std::tolower(major.at(i)) != 'g')
            return false;
    }
    return true;
}

void TraCITrafficLightOAFLogic::logPlatoonConfig(const std::string& colorWord, const signalScheme& signal,
        const phasetime_t& greenTime)
{
    customLog << simTime().inUnit(SIMTIME_MS) << ";"
              << colorWord << ";"
              << greenTime.dbl() << ";"
              << signal << ";"
              << "'";
    for (auto&& itPair : scheduledConfig) {
        customLog << itPair.first
                  << "=" << itPair.second
                  << "(" << scheduler.estimateGreenTime(itPair.first, itPair.second).dbl() << ")"
                  << ",";
    }
    customLog << "'" << std::endl;
    customLog.flush();
}

void TraCITrafficLightOAFLogic::highlightScheduledConfig() {
    auto&& scenarioManager(TraCIScenarioManagerAccess().get());
    auto&& allVehicles(scenarioManager->getManagedHosts());
    auto&& iface(scenarioManager->getCommandInterface());
    for(auto&& itApproachCfg : scheduledConfig) {
        auto vehicles = scheduler.vehiclesOnApproach(itApproachCfg.first);
        size_t i = 0;
        for(auto&& itVehicle : vehicles) {
            if(allVehicles.find(itVehicle.vehicleId) == allVehicles.end()) {
                scheduler.clearVehicle(itVehicle.vehicleId);
                continue;
            }
            iface->vehicle(itVehicle.vehicleId).setColor({64, 64, 64, 0});
            i++;
            if (i >= itApproachCfg.second)
                break;
        }
    }
}
