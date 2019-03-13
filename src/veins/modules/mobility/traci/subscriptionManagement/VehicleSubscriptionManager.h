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

#include <set>
#include <string>
#include <vector>
#include <list>
#include <memory>

#include "veins/veins.h"

#include "veins/modules/mobility/traci/subscriptionManagement/SubscriptionManagerBase.h"
#include "veins/modules/mobility/traci/subscriptionManagement/Vehicle.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class VehicleSubscriptionManager
 *
 * Manages all vehicle subscriptions. You can update it via a subscription
 * result in the form of a TraCIBuffer or via a manual update but it is
 * necessary to provide an id_list of active vehicles then.
 *
 * After updating you can request the disappeared and appeared vehicles via
 * the class methods.
 *
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class VehicleSubscriptionManager : public SubscriptionManagerBase {
public:
    /**
     * Constructor.
     *
     * @param connection to the traci server to perform subscriptions.
     * @param commandInterface to the traci server to request variables.
     */
    VehicleSubscriptionManager(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface);

    /**
     * Default destructor.
     */
    virtual ~VehicleSubscriptionManager() = default;

    /**
     * Perform an explicit update of this manager.
     *
     * If you dont receive id_list subscription you can force updates of the internal
     * manager state using this function. This essentially does the same as the
     * update() method when the buffer supplied to it contains an id list subscription.
     *
     * This method takes the currently active vehicle ids supplied with the call to this
     * method and compares it with the current subscribed vehicle. Based on that list
     * it will calculate which vehicles disappeared and appeared.
     *
     * Note: The param is a list because that is what is supplied when using the command
     * interface.
     *
     * @param currentlyVehiclePersonIds A list containing the currently active vehicles
     * identified by their ids.
     *
     */
    void updateWithList(std::list<std::string>& currentlyActiveVehicleIds);

    bool update(TraCIBuffer& buffer);

    /**
     * This gives you all the vehicles that were updated (includes
     * new vehicles) since the last time you called clearAPI().
     *
     * This can also contain multiple updates for a single vehicle if
     * you dont call clearAPI() regularly.
     *
     * @return a vector of TraCIVehicle.
     */
    std::vector<Vehicle> getUpdated() const;

    /**
     * This gives you all the disappeared vehicles since you called
     * this method the last time. That means that multiple calls
     * to this method will yield different results.
     *
     * @return std::set<std::string> a set of vehicle ids.
     */
    std::vector<std::string> getDisappeared() const;

    /**
     * Clear the API.
     *
     * In this case it will clear class variables that will
     * affect calls to:
     *  - getUpdated()
     *  - getDisappeared()
     */
    void clearAPI() override;

private:
    /**
     * This contains all updated vehicles. Will be cleared after
     * getUpdated() was called.
     */
    std::vector<Vehicle> updatedVehicles;

    /**
     * This contains all disappeared vehicles. Will be cleared after
     * clearAPI() was called.
     */
    std::set<std::string> disappearedVehicles;

    /**
     * Process a subscription result that contains an id list. This is
     * simply a helper method to improve clarity of update().
     */
    void processVehicleIDList(std::list<std::string>& idList);

    /**
     * Subscribe to specific TraCI vehicle variables.
     *
     * @param id the id of the vehicle.
     */
    void subscribeToVehicleVariables(std::string id);

    /**
     * Subscribe to the vehicle id list and evaluate the response.
     */
    void subscribeToVehicleIDList();
};

} // end namespace TraCISubscriptionManagement
} // namespace veins
