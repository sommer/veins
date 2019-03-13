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

#pragma once

#include "veins/veins.h"

#include "veins/modules/mobility/traci/subscriptionManagement/PersonSubscriptionManager.h"
#include "veins/modules/mobility/traci/subscriptionManagement/SimulationSubscriptionManager.h"
#include "veins/modules/mobility/traci/subscriptionManagement/Person.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TrafficLight.h"
#include "veins/modules/mobility/traci/subscriptionManagement/Vehicle.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TrafficLightSubscriptionManager.h"
#include "veins/modules/mobility/traci/subscriptionManagement/VehicleSubscriptionManager.h"
#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class ExecutiveSubscriptionManager
 *
 * This manages all the subscriptions that were made and provides an API
 * to the underlying subscription managers and request their results.
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class ExecutiveSubscriptionManager {

public:
    /**
     * Constructor.
     *
     * @param connection the connection to the traci server to perform subscripitons
     * @param commandInterface the command interface to make requests to the traci server
     * @param explicitUpdateIfSubscriptionUnavailable In case there is no subscription response
     * for a certain type of participant it will be replaced with an explicit request.
     */
    ExecutiveSubscriptionManager(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface, bool explicitUpdateIfSubscriptionUnavailable = true);

    /**
     * Process a subscription result.
     *
     * Do not modify the buffer before inserting it into this method.
     *
     * This updates the internal state of this object and changes
     * responses to other methods of this object.
     *
     * @param buffer TraCIBuffer containing subscriptions.
     */
    void processSubscriptionResult(TraCIBuffer& buffer);

    /**
     * Returns a list of persons that are new or updated. It is up
     * to the caller to decide which person is updated and which is
     * new.
     *
     * This gives you a list that contains all updated person since
     * you called clearAPI() the last time.
     *
     * @return vector<TraCIPerson> a list of persons.
     */
    std::vector<Person> getUpdatedPersons() const;

    /**
     * Returns a set of persons that disappeared since the last time
     * you called clearAPI().
     *
     * @return set<string> a set of disappeared person ids.
     */
    std::vector<std::string> getDisappearedPersons() const;

    /**
     * Returns a list of vehicle that are new or updated. It is up
     * to the caller to decide which vehicle is updated and which is
     * new.
     *
     * This gives you a list that contains all updated vehicles since
     * you called clearAPI() the last time.
     *
     * @return vector<Vehicle> a list of vehicles.
     */
    std::vector<Vehicle> getUpdatedVehicles() const;

    /**
     * Returns a set of vehicles that disappeared since the last time
     * you called clearAPI().
     *
     * @return vector<string> a set of disappeared vehicle ids.
     */
    std::vector<std::string> getDisappearedVehicles() const;

    /**
     * Get the ids of vehicles that started teleporting since
     * the last time you called clearAPI().
     *
     * @return vector<string> a list of ids that started teleporting.
     */
    std::vector<std::string> getStartedTeleporting() const;

    /**
     * Get the ids of vehicles that started parking since
     * the last time you called clearAPI().
     *
     * @return vector<string> a list of ids that started parking.
     */
    std::vector<std::string> getStartedParking() const;

    /**
     * Get the ids of vehicles that ended parking since
     * the last time you called clearAPI().
     *
     * @return vector<string> a list of ids that ended parking.
     */
    std::vector<std::string> getEndedParking() const;

    /**
     * Gives a list of traffic light subscription updates since the last
     * time clearAPI() was called.
     *
     * @return std::vector<TrafficLight> list of traffic light updates.
     */
    std::vector<TrafficLight> getTrafficLightUpdates() const;

    /**
     * Subscribes to the following variables of a traffic light:
     *      - current phase (0x28)
     *      - current program (0x29)
     *      - next switch (0x2d)
     *      - red yellow green state (0x20)
     *
     * @param id of the traffic light to subscribe to.
     */
    void subscribeToTrafficLight(std::string id);

    /**
     * Initialize this manager with the given parameters to access the traci server.
     */
    void initialize(std::shared_ptr<TraCIConnection> connection, std::shared_ptr<TraCICommandInterface> commandInterface);

    /**
     * Clear API.
     *
     * This will clear the underlying containers which influence the
     * results of the following methods:
     *  - getUpdatedPersons()
     *  - getDisappearedPersons()
     *  - getUpdatedVehicles()
     *  - getDisappearedVehicles()
     *  - getStartedTeleporting()
     *  - getStartedParking()
     *  - getEndedParking()
     *  - getTrafficLightUpdates()
     */
    void clearAPI();

private:

    /**
     * The command interface to the TraCI server.
     */
    std::shared_ptr<TraCICommandInterface> commandInterface;

    /**
     * Stores if the id lists of participants should be updated with an explicit request
     * if there is no subscription response included.
     */
    bool explicitUpdateIfIdListSubscriptionUnavailable;

    /**
     * Stores the person subscription manager.
     */
    PersonSubscriptionManager personSubscriptionManager;

    /**
     * Stores the vehicle subscription manager.
     */
    VehicleSubscriptionManager vehicleSubscriptionManager;

    /**
     * Stores the simulation subscription manager.
     */
    SimulationSubscriptionManager simulationSubscriptionManager;

    /**
     * Stores the traffic light subscription manager.
     */
    TrafficLightSubscriptionManager trafficLightSubscriptionManager;
};

} // end namespace TraCISubscriptionManagement
} // namespace veins
