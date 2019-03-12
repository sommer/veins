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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_VEHICLESUBSCRIPTIONMANAGER_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_VEHICLESUBSCRIPTIONMANAGER_H_

#include <set>
#include <string>
#include <list>
#include <memory>

#include "veins/modules/mobility/traci/subscriptionManagement/SubscriptionManagerBase.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCIVehicle.h"

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
 * IMPORTANT: Call initialize() before using!
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class VehicleSubscriptionManager: public SubscriptionManagerBase {
public:
    /**
     * Constructor.
     */
    VehicleSubscriptionManager();

    /**
     * Default destructor.
     */
    virtual ~VehicleSubscriptionManager() = default;

    /**
     * Update this VehicleSubscriptionManager with the given
     * active vehicle id list. You can force an update of active
     * vehicles using this.
     *
     * @param currentlyActiveVehicleIds A list containing the currently active
     * vehicles identified by their ids.
     */
    void updateWithList(std::list<std::string>& currentlyActiveVehicleIds);

    /**
     * Update this manager with the given buffer containing a subscription
     * response for a vehicle value.
     *
     * This will create new subscriptions for specific vehicles if the buffer
     * contained an ID_LIST subscription.
     *
     * @param buffer the traci buffer containing the subscription response.
     *
     * @return bool true if the given buffer contained an ID_LIST response.
     */
    bool update(TraCIBuffer& buffer);

    /**
     * This gives you all the vehicles that were updated (includes
     * new vehicles) since the last time you called this method. That
     * means that multiple calls to this method will NOT yield the same
     * result.
     *
     * This can also contain multiple updates for a single vehicle.
     *
     * @return a list of TraCIVehicle.
     */
    std::list<TraCIVehicle> getUpdated();

    /**
     * This gives you all the disappeared vehicles since you called
     * this method the last time. That means that multiple calls
     * to this method will yield different results.
     *
     * @return std::set<std::string> a set of vehicle ids.
     */
    std::set<std::string> getDisappeared();

    /**
    * Initialize this manager with the given parameters to access TraCI.
    */
    void initialize(std::shared_ptr<TraCIConnection> connection, std::shared_ptr<TraCICommandInterface> commandInterface);

private:

    /**
     * This contains all updated vehicles. Will be cleared after
     * getUpdated() was called.
     */
    std::list<TraCIVehicle> mUpdatedVehicles;

    /**
     * This is the difference between mSubscribedVehicleIds and
     * currently active vehicles (provided with call to update()).
     *
     * This takes all the elements which are contained in
     * mSubscribedVehicleIds but not in currently active vehicles ids.
     */
    std::set<std::string> mDisappearedVehicles;

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

};

} // end namespace TraCISubscriptionManagement
} // end namespace Veins

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_VEHICLESUBSCRIPTIONMANAGER_H_ */
