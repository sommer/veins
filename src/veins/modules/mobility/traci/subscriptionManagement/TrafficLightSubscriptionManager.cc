//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
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

#include "veins/modules/mobility/traci/subscriptionManagement/TrafficLightSubscriptionManager.h"
#include "veins/veins.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"

namespace veins {
namespace TraCISubscriptionManagement {

TrafficLightSubscriptionManager::TrafficLightSubscriptionManager()
    : SubscriptionManagerBase()
    , mUpdatedTrafficLights()
{
}

void TrafficLightSubscriptionManager::subscribeToTrafficLight(std::string id) {
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = id;
    uint8_t variableNumber = 4;
    uint8_t variable1 = TraCIConstants::TL_CURRENT_PHASE;
    uint8_t variable2 = TraCIConstants::TL_CURRENT_PROGRAM;
    uint8_t variable3 = TraCIConstants::TL_NEXT_SWITCH;
    uint8_t variable4 = TraCIConstants::TL_RED_YELLOW_GREEN_STATE;

    TraCIBuffer buffer = getConnection()->query(TraCIConstants::CMD_SUBSCRIBE_TL_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4);

    // remove unnecessary stuff from buffer
    uint8_t responseCommandLength;
    buffer >> responseCommandLength;
    ASSERT(responseCommandLength == 0);
    // this is the length of the command
    uint32_t responseCommandLengthExtended;
    buffer >> responseCommandLengthExtended;
    uint8_t responseCommandID;
    buffer >> responseCommandID;
    ASSERT(responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_TL_VARIABLE);

    update(buffer);

    ASSERT(buffer.eof());

    // everything fine so add to subscribed
    addToSubscribed(id);
}

std::list<TraCITrafficLight> TrafficLightSubscriptionManager::getUpdated() {
    std::list<TraCITrafficLight> temp = mUpdatedTrafficLights;
    mUpdatedTrafficLights.clear();
    return temp;
}

bool TrafficLightSubscriptionManager::update(TraCIBuffer& buffer) {
    // this is the object id that this subscription result contains
    // content about. in this case we expect a traffic light id.
    std::string responseObjectID;
    buffer >> responseObjectID;

    // the number of response variables that are contained in this buffer
    uint8_t numberOfResponseVariables;
    buffer >> numberOfResponseVariables;
    // this should be 4
    ASSERT(numberOfResponseVariables == 4);

    // this needs to be filled
    TraCITrafficLight trafficLight;

    for (int counter = 0; counter < numberOfResponseVariables; ++counter) {
        // extract a couple of values:
        // - identifies the variable (position, list etc.)
        uint8_t responseVariableID;
        buffer >> responseVariableID;
        // - status of the variable
        uint8_t variableStatus;
        buffer >> variableStatus;

        // check ok
        if (variableStatus == TraCIConstants::RTYPE_OK) {

            switch (responseVariableID) {
            case TraCIConstants::TL_CURRENT_PHASE:
                trafficLight.currentPhase = buffer.readTypeChecked<int32_t>(TraCIConstants::TYPE_INTEGER);
                break;

            case TraCIConstants::TL_CURRENT_PROGRAM:
                trafficLight.currentProgram = buffer.readTypeChecked<std::string>(TraCIConstants::TYPE_STRING);
                break;

            case TraCIConstants::TL_NEXT_SWITCH:
                trafficLight.nextSwitch = buffer.readTypeChecked<simtime_t>(getCommandInterface()->getTimeType());

                break;

            case TraCIConstants::TL_RED_YELLOW_GREEN_STATE:
                        trafficLight.redYellowGreenState =  buffer.readTypeChecked<std::string>(TraCIConstants::TYPE_STRING);
                break;

            default:
                throw cRuntimeError("Received unhandled traffic light subscription result; type: 0x%02x", responseVariableID);
                break;
            }

        } else {
            // - type of the variable
            uint8_t variableType;
            buffer >> variableType;
            // the status of the variable is not ok
            ASSERT(variableType == TraCIConstants::TYPE_STRING);
            std::string errormsg;
            buffer >> errormsg;
            if (isSubscribed(responseObjectID)) {
                if (variableStatus == TraCIConstants::RTYPE_NOTIMPLEMENTED) {
                    throw cRuntimeError(
                            "TraCI server reported subscribing to vehicle variable 0x%2x not implemented (\"%s\"). Might need newer version.",
                            responseVariableID, errormsg.c_str());
                }

                throw cRuntimeError(
                        "TraCI server reported error subscribing to vehicle variable 0x%2x (\"%s\").",
                        responseVariableID, errormsg.c_str());
            }
        }

    }

    trafficLight.id = responseObjectID;
    mUpdatedTrafficLights.push_back(trafficLight);

    return true;
}

} // end namespace TraCISubscriptionManagement
} /* namespace Veins */
