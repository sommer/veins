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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACISUBSCRIPTIONMANAGER_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACISUBSCRIPTIONMANAGER_H_

#include "veins/modules/mobility/traci/subscriptionManagement/PersonSubscriptionManager.h"
#include "veins/modules/mobility/traci/subscriptionManagement/SimulationSubscriptionManager.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCIPerson.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCITrafficLight.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCIVehicle.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TrafficLightSubscriptionManager.h"
#include "veins/modules/mobility/traci/subscriptionManagement/VehicleSubscriptionManager.h"
#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class TraCISubscriptionManager
 *
 * This manages all the subscriptions that were made and provides an API
 * to the underlying subscription managers and request their results.
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class TraCISubscriptionManager {

public:

    /**
     * Default constructor.
     *
     * @param explicitUpdateIfSubscriptionUnavailable In case there is no subscription response
     * for a certain type of participant it will be replaced with an explicit request.
     */
    TraCISubscriptionManager(bool explicitUpdateIfSubscriptionUnavailable = true);

    /**
     * Default destructor.
     */
    virtual ~TraCISubscriptionManager() = default;

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
     * Multiple calls to this method will not yield the same result. It
     * will always give you the new or updated persons since the last
     * time you called this method.
     *
     * @return list<TraCIPerson> a list of persons.
     */
    std::list<TraCIPerson> getUpdatedPersons();

    /**
     * Returns a set of persons that disappeared since the last time
     * you called this method.
     *
     * @return set<string> a set of disappeared person ids.
     */
    std::set<std::string> getDisappearedPersons();

    /**
     * Returns a list of vehicle that are new or updated. It is up
     * to the caller to decide which vehicle is updated and which is
     * new.
     *
     * Multiple calls to this method will not yield the same result. It
     * will always give you the new or updated vehicles since the last
     * time you called this method.
     *
     * @return list<TraCIVehicle> a list of vehicles.
     */
    std::list<TraCIVehicle> getUpdatedVehicles();

    /**
     * Returns a set of vehicles that disappeared since the last time
     * you called this method.
     *
     * @return set<string> a set of disappeared vehicle ids.
     */
    std::set<std::string> getDisappearedVehicles();


    /**
     * Get the ids of vehicles that started teleporting since
     * the last time you called this method.
     *
     * Note: Multiple calls to this method will not yield the
     * same result.
     *
     * @return list<string> a list of ids that started teleporting.
     */
    std::list<std::string> getStartedTeleporting();

    /**
     * Get the ids of vehicles that started parking since
     * the last time you called this method.
     *
     * Note: Multiple calls to this method will not yield the
     * same result.
     *
     * @return list<string> a list of ids that started parking.
     */
    std::list<std::string> getStartedParking();

    /**
     * Get the ids of vehicles that ended parking since
     * the last time you called this method.
     *
     * Note: Multiple calls to this method will not yield the
     * same result.
     *
     * @return list<string> a list of ids that ended parking.
     */
    std::list<std::string> getEndedParking();

    /**
     * Gives a list of traffic light subscription updates since the last
     * time this method was called.
     *
     * @return std::list<TraCITrafficLight> list of traffic light updates.
     */
    std::list<TraCITrafficLight> getTrafficLightUpdates();

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

private:

    /**
     * The connection to the TraCI server.
     */
    std::shared_ptr<TraCIConnection> mConnection;

    /**
     * The command interface to the TraCI server.
     */
    std::shared_ptr<TraCICommandInterface> mCommandInterface;

    /**
     * Stores if the id lists of participants should be updated with an explicit request
     * if there is no subscription response included.
     */
    bool mExplicitUpdateIfIdListSubscriptionUnavailable;

    /**
     * Stores the person subscription manager.
     */
    PersonSubscriptionManager mPersonSubscriptionManager;

    /**
     * Stores the vehicle subscription manager.
     */
    VehicleSubscriptionManager mVehicleSubscriptionManager;

    /**
     * Stores the simulation subscription manager.
     */
    SimulationSubscriptionManager mSimulationSubscriptionManager;

    /**
     * Stores the traffic light subscription manager.
     */
    TrafficLightSubscriptionManager mTrafficLightSubscriptionManager;

};

} // end namespace TraCISubscriptionManagement
} // end namespace Veins

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_TRACISUBSCRIPTIONMANAGER_H_ */
