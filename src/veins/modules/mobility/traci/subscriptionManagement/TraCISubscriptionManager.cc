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

#include "veins/modules/mobility/traci/subscriptionManagement/TraCISubscriptionManager.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"

namespace veins {
namespace TraCISubscriptionManagement {

TraCISubscriptionManager::TraCISubscriptionManager(bool explicitUpdateIfIdListSubscriptionUnavailable)
    : mConnection(nullptr)
    , mCommandInterface(nullptr)
    , mExplicitUpdateIfIdListSubscriptionUnavailable(explicitUpdateIfIdListSubscriptionUnavailable)
    , mPersonSubscriptionManager()
    , mVehicleSubscriptionManager()
    , mSimulationSubscriptionManager()
    , mTrafficLightSubscriptionManager()
{
}

void TraCISubscriptionManager::processSubscriptionResult(TraCIBuffer& buffer) {

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
            receivedVehicleIDListSubscription =
                    mVehicleSubscriptionManager.update(buffer) || receivedVehicleIDListSubscription;
        } else if (responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_PERSON_VARIABLE) {
            receivedPersonIDListSubscription =
                    mPersonSubscriptionManager.update(buffer)  || receivedPersonIDListSubscription;
        } else if (responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_SIM_VARIABLE)
            mSimulationSubscriptionManager.update(buffer);
        else if (responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_TL_VARIABLE)
            mTrafficLightSubscriptionManager.update(buffer);
        else {
            throw cRuntimeError("Received unknown subscription result!");
        }

    }

    // NOTE: This is only done because I was not able to receive a person ID LIST subscription
    // If it is somehow possible to get a person id list subscription working. This can be removed.

    // if there was no person id list received we perform a manual update
    if (!receivedPersonIDListSubscription && mExplicitUpdateIfIdListSubscriptionUnavailable) {
        veins::TraCICommandInterface::Person defaultPerson = mCommandInterface->person("");
        std::list<std::string> idList = defaultPerson.getIdList();
        mPersonSubscriptionManager.updateWithList(idList);
    }
    // if there was no vehicle id list received we perform a manual update
    if (!receivedVehicleIDListSubscription && mExplicitUpdateIfIdListSubscriptionUnavailable) {
        veins::TraCICommandInterface::Vehicle defaultVehicle = mCommandInterface->vehicle("");
        std::list<std::string> idList = defaultVehicle.getIdList();
        mVehicleSubscriptionManager.updateWithList(idList);
    }

    ASSERT(buffer.eof());

}

std::list<TraCIPerson> TraCISubscriptionManager::getUpdatedPersons() {
    // simply delegate
    return mPersonSubscriptionManager.getUpdated();
}

std::set<std::string> TraCISubscriptionManager::getDisappearedPersons() {
    // simply delegate
    return mPersonSubscriptionManager.getDisappeared();
}

std::list<TraCIVehicle> TraCISubscriptionManager::getUpdatedVehicles() {
    // simply delegate
    return mVehicleSubscriptionManager.getUpdated();
}

std::set<std::string> TraCISubscriptionManager::getDisappearedVehicles() {
    // simply delegate
    return mVehicleSubscriptionManager.getDisappeared();
}

std::list<std::string> TraCISubscriptionManager::getStartedTeleporting() {
    return mSimulationSubscriptionManager.getStartedTeleporting();
}

std::list<std::string> TraCISubscriptionManager::getStartedParking() {
    return mSimulationSubscriptionManager.getStartedParking();
}

std::list<std::string> TraCISubscriptionManager::getEndedParking() {
    return mSimulationSubscriptionManager.getEndedParking();
}

std::list<TraCITrafficLight> TraCISubscriptionManager::getTrafficLightUpdates() {
    return mTrafficLightSubscriptionManager.getUpdated();
}

void TraCISubscriptionManager::subscribeToTrafficLight(std::string id) {
    mTrafficLightSubscriptionManager.subscribeToTrafficLight(id);
}

void TraCISubscriptionManager::initialize(std::shared_ptr<TraCIConnection> connection, std::shared_ptr<TraCICommandInterface> commandInterface) {
    mConnection = connection;
    mCommandInterface = commandInterface;
    // call subscription managers init
    mPersonSubscriptionManager.initialize(connection, commandInterface);
    mVehicleSubscriptionManager.initialize(connection, commandInterface);
    mSimulationSubscriptionManager.initialize(connection, commandInterface);
    mTrafficLightSubscriptionManager.initialize(connection, commandInterface);
}

} // end namespace TraCISubscriptionManagement
} // end namespace Veins

