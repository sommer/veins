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

#include "veins/modules/mobility/traci/subscriptionManagement/ExecutiveSubscriptionManager.h"

#include "veins/modules/mobility/traci/TraCIConstants.h"

namespace veins {
namespace TraCISubscriptionManagement {

ExecutiveSubscriptionManager::ExecutiveSubscriptionManager(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface, bool explicitUpdateIfIdListSubscriptionUnavailable)
    : commandInterface(commandInterface)
    , explicitUpdateIfIdListSubscriptionUnavailable(explicitUpdateIfIdListSubscriptionUnavailable)
    , personSubscriptionManager(connection, commandInterface)
    , vehicleSubscriptionManager(connection, commandInterface)
    , simulationSubscriptionManager(connection, commandInterface)
    , trafficLightSubscriptionManager(connection, commandInterface)
{
}

void ExecutiveSubscriptionManager::processSubscriptionResult(TraCIBuffer& buffer)
{

    bool receivedPersonIDListSubscription = false;
    bool receivedVehicleIDListSubscription = false;

    uint32_t subscriptionResultCount;
    buffer >> subscriptionResultCount;
    EV_DEBUG << "Getting " << subscriptionResultCount << " subscription results" << endl;
    for (uint32_t i = 0; i < subscriptionResultCount; ++i) {

        // this should be zero
        uint8_t responseCommandLength;
        buffer >> responseCommandLength;
        ASSERT(responseCommandLength == 0);
        // this is the length of the command
        uint32_t responseCommandLengthExtended;
        buffer >> responseCommandLengthExtended;
        // this is the response command identifier
        // with this we can find out what kind of subscription result
        // we receive (person subscription or vehicle subscription etc.)
        uint8_t responseCommandID;
        buffer >> responseCommandID;

        if (responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE) {
            receivedVehicleIDListSubscription = vehicleSubscriptionManager.update(buffer) || receivedVehicleIDListSubscription;
        }
        else if (responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_PERSON_VARIABLE) {
            receivedPersonIDListSubscription = personSubscriptionManager.update(buffer) || receivedPersonIDListSubscription;
        }
        else if (responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_SIM_VARIABLE)
            simulationSubscriptionManager.update(buffer);
        else if (responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_TL_VARIABLE)
            trafficLightSubscriptionManager.update(buffer);
        else {
            throw cRuntimeError("Received unknown subscription result!");
        }
    }

    // NOTE: This is only done because I was not able to receive a person ID LIST subscription
    // If it is somehow possible to get a person id list subscription working. This can be removed.

    // if there was no person id list received we perform a manual update
    if (!receivedPersonIDListSubscription && explicitUpdateIfIdListSubscriptionUnavailable) {
        veins::TraCICommandInterface::Person defaultPerson = commandInterface->person("");
        std::list<std::string> idList = defaultPerson.getIdList();
        personSubscriptionManager.updateWithList(idList);
    }
    // if there was no vehicle id list received we perform a manual update
    if (!receivedVehicleIDListSubscription && explicitUpdateIfIdListSubscriptionUnavailable) {
        veins::TraCICommandInterface::Vehicle defaultVehicle = commandInterface->vehicle("");
        std::list<std::string> idList = defaultVehicle.getIdList();
        vehicleSubscriptionManager.updateWithList(idList);
    }

    ASSERT(buffer.eof());
}

std::vector<Person> ExecutiveSubscriptionManager::getUpdatedPersons() const
{
    // simply delegate
    return personSubscriptionManager.getUpdated();
}

std::vector<std::string> ExecutiveSubscriptionManager::getDisappearedPersons() const
{
    // simply delegate
    return personSubscriptionManager.getDisappeared();
}

std::vector<Vehicle> ExecutiveSubscriptionManager::getUpdatedVehicles() const
{
    // simply delegate
    return vehicleSubscriptionManager.getUpdated();
}

std::vector<std::string> ExecutiveSubscriptionManager::getDisappearedVehicles() const
{
    // simply delegate
    return vehicleSubscriptionManager.getDisappeared();
}

std::vector<std::string> ExecutiveSubscriptionManager::getStartedTeleporting() const
{
    return simulationSubscriptionManager.getStartedTeleporting();
}

std::vector<std::string> ExecutiveSubscriptionManager::getStartedParking() const
{
    return simulationSubscriptionManager.getStartedParking();
}

std::vector<std::string> ExecutiveSubscriptionManager::getEndedParking() const
{
    return simulationSubscriptionManager.getEndedParking();
}

std::vector<TrafficLight> ExecutiveSubscriptionManager::getTrafficLightUpdates() const
{
    return trafficLightSubscriptionManager.getUpdated();
}

void ExecutiveSubscriptionManager::subscribeToTrafficLight(std::string id)
{
    trafficLightSubscriptionManager.subscribe(id);
}

void ExecutiveSubscriptionManager::clearAPI() {
    personSubscriptionManager.clearAPI();
    vehicleSubscriptionManager.clearAPI();
    simulationSubscriptionManager.clearAPI();
    trafficLightSubscriptionManager.clearAPI();
}

} // end namespace TraCISubscriptionManagement
} // namespace veins
