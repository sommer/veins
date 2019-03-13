//
// Copyright (C) 2006-2017 Nico Dassler <dassler@hm.edu>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/mobility/traci/subscriptionManagement/SimulationSubscriptionManager.h"

#include "veins/modules/mobility/traci/TraCIConstants.h"

namespace veins {
namespace TraCISubscriptionManagement {

SimulationSubscriptionManager::SimulationSubscriptionManager(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface)
    : SubscriptionManagerBase(connection, commandInterface)
    , startedTeleporting()
    , startedParking()
    , endedParking()
{
    subscribeToSimulationVariables();
}

bool SimulationSubscriptionManager::update(TraCIBuffer& buffer)
{

    // this is the object id that this subscription result contains
    // content about. this is not used for this subscription.
    std::string responseObjectID;
    buffer >> responseObjectID;

    uint8_t variableNumber_resp;
    buffer >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j) {
        uint8_t variable1_resp;
        buffer >> variable1_resp;
        uint8_t isokay;
        buffer >> isokay;
        if (isokay != TraCIConstants::RTYPE_OK) {
            uint8_t varType;
            buffer >> varType;
            ASSERT(varType == TraCIConstants::TYPE_STRING);
            std::string description;
            buffer >> description;
            if (isokay == TraCIConstants::RTYPE_NOTIMPLEMENTED)
                throw cRuntimeError("TraCI server reported subscribing to variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, description.c_str());
            cRuntimeError("TraCI server reported error subscribing to variable 0x%2x (\"%s\").", variable1_resp, description.c_str());
        }

        if (variable1_resp == TraCIConstants::VAR_TELEPORT_STARTING_VEHICLES_IDS) {
            uint8_t varType;
            buffer >> varType;
            ASSERT(varType == TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buffer >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles starting to teleport." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buffer >> idstring;
                startedTeleporting.push_back(idstring);
            }
        }
        else if (variable1_resp == TraCIConstants::VAR_PARKING_STARTING_VEHICLES_IDS) {
            uint8_t varType;
            buffer >> varType;
            ASSERT(varType == TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buffer >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles starting to park." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buffer >> idstring;
                startedParking.push_back(idstring);
            }
        }
        else if (variable1_resp == TraCIConstants::VAR_PARKING_ENDING_VEHICLES_IDS) {
            uint8_t varType;
            buffer >> varType;
            ASSERT(varType == TraCIConstants::TYPE_STRINGLIST);
            uint32_t count;
            buffer >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles ending to park." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buffer >> idstring;
                endedParking.push_back(idstring);
            }
        }
        else if (variable1_resp == getCommandInterface()->getTimeStepCmd()) {
            uint8_t varType;
            buffer >> varType;
            ASSERT(varType == getCommandInterface()->getTimeType());
            simtime_t serverTimestep;
            buffer >> serverTimestep;
            EV_DEBUG << "TraCI reports current time step as " << serverTimestep << "ms." << endl;
            simtime_t omnetTimestep = simTime();
            ASSERT(omnetTimestep == serverTimestep);
        }
        else {
            throw cRuntimeError("Received unhandled simulation subscription result");
        }
    }
    return true;
}

std::vector<std::string> SimulationSubscriptionManager::getStartedTeleporting() const
{
    return startedTeleporting;
}

std::vector<std::string> SimulationSubscriptionManager::getStartedParking() const
{
    return startedParking;
}

std::vector<std::string> SimulationSubscriptionManager::getEndedParking() const
{
    return endedParking;
}

void SimulationSubscriptionManager::clearAPI() {
    startedParking.clear();
    startedTeleporting.clear();
    endedParking.clear();
}

void SimulationSubscriptionManager::subscribeToSimulationVariables() {
    // subscribe to some simulation variables
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = "";
    uint8_t variableNumber = 4;
    uint8_t variable1 = getCommandInterface()->getTimeStepCmd();
    uint8_t variable2 = TraCIConstants::VAR_TELEPORT_STARTING_VEHICLES_IDS;
    uint8_t variable3 = TraCIConstants::VAR_PARKING_STARTING_VEHICLES_IDS;
    uint8_t variable4 = TraCIConstants::VAR_PARKING_ENDING_VEHICLES_IDS;
    TraCIBuffer buffer = getConnection()->query(TraCIConstants::CMD_SUBSCRIBE_SIM_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4);

    // remove unnecessary stuff from buffer
    uint8_t responseCommandLength;
    buffer >> responseCommandLength;
    ASSERT(responseCommandLength == 0);
    // this is the length of the command
    uint32_t responseCommandLengthExtended;
    buffer >> responseCommandLengthExtended;
    uint8_t responseCommandID;
    buffer >> responseCommandID;
    ASSERT(responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_SIM_VARIABLE);

    update(buffer);

    ASSERT(buffer.eof());
}

} // end namespace TraCISubscriptionManagement
} // namespace veins
