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

#include <vector>
#include <string>

#include "veins/veins.h"

#include "veins/modules/mobility/traci/subscriptionManagement/SubscriptionManagerBase.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class SimulationSubscriptionManager
 *
 * This checks some of the simulation variables and ensures that everything
 * is right. For example: Checking if the simulation time is the expected value.
 *
 * It also offers the possibility to request parking and teleporting vehicles
 * with the class methods.
 *
 * Updates are only possible with subscriptions via the update() method.
 *
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class SimulationSubscriptionManager : public SubscriptionManagerBase {
public:
    /**
     * Constructor.
     *
     * @param connection to the traci server to perform subscriptions.
     * @param commandInterface to the traci server to request variables.
     */
    SimulationSubscriptionManager(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface);

    /**
     * Default destructor.
     */
    virtual ~SimulationSubscriptionManager() = default;

    /**
     * Update this subscription manager with the given TraCIBuffer. After a call to
     * this the getter of this class will be updated and return different results than
     * before the call to update().
     *
     * @param buffer the buffer containing the subscription result.
     *
     * @return true (default)
     */
    bool update(TraCIBuffer& buffer) override;

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
     * Clears the api.
     *
     * Clears the underlying containers to methods:
     *  - getStartedTeleporting()
     *  - getStartedParking()
     *  - getEndedParking()
     */
    void clearAPI() override;

private:
    /**
     * Stores the ids that started teleporting since the last time
     * clearAPI() was called.
     */
    std::vector<std::string> startedTeleporting;

    /**
     * Stores the ids that started parking since the last time
     * clearAPI() was called.
     */
    std::vector<std::string> startedParking;

    /**
     * Stores the ids that ended Parking since the last time
     * clearAPI() was called.
     */
    std::vector<std::string> endedParking;

    /**
     * Subscribe to simulation variables and evaluate the response.
     *
     * The following simulation values are subscribed:
     *  - TraCIConstants::VAR_TELEPORT_STARTING_VEHICLES_IDS
     *  - TraCIConstants::VAR_PARKING_STARTING_VEHICLES_IDS;
     *  - TraCIConstants::VAR_PARKING_ENDING_VEHICLES_IDS;
     */
    void subscribeToSimulationVariables();

};

} // end namespace TraCISubscriptionManagement
} // namespace veins
