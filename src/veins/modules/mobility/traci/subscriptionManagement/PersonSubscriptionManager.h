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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_PERSONSUBSCRIPTIONMANAGER_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_PERSONSUBSCRIPTIONMANAGER_H_

#include <set>
#include <string>
#include <list>

#include "veins/modules/mobility/traci/subscriptionManagement/SubscriptionManagerBase.h"
#include "veins/modules/mobility/traci/subscriptionManagement/TraCIPerson.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class PersonSubscriptionManager
 *
 * Manages all person subscriptions. You can update it via a subscription
 * result in the form of a TraCIBuffer or via a manual update but it is
 * necessary to provide an id_list of active persons then.
 *
 * After updating you can request the disappeared and appeared persons via
 * the class methods.
 *
 * IMPORTANT: Call initialize() before using!
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class PersonSubscriptionManager: public SubscriptionManagerBase {

public:
    /**
     * Constructor.
     */
    PersonSubscriptionManager();

    /**
     * Default destructor.
     */
    virtual ~PersonSubscriptionManager() = default;

    /**
     * Update this PersonSubscriptionManager with the given
     * active person id list. You can force an update of active
     * persons using this.
     *
     * @param currentlyActivePersonIds A list containing the currently active persons
     * identified by their ids.
     *
     * @return true
     */
    bool updateWithList(std::list<std::string>& currentlyActivePersonIds);

    /**
     * Update this manager with the given buffer containing a subscription
     * response for a person value.
     *
     * This will create new subscriptions for specific persons if the buffer
     * contained an ID_LIST subscription.
     *
     * @param buffer the traci buffer containing the subscription response.
     *
     * @return bool true if the given buffer contained an ID_LIST response.
     */
    bool update(TraCIBuffer& buffer) override;

    /**
     * This gives you all the persons that were updated (includes
     * new persons) since the last time you called this method. That
     * means that multiple calls to this method will NOT yield the same
     * result.
     *
     * This can also contain multiple updates for a single person.
     *
     * @return a list of TraCIPerson.
     */
    std::list<TraCIPerson> getUpdated();

    /**
     * This gives you all the disappeared persons since you called
     * this method the last time. That means that multiple calls
     * to this method will yield different results.
     *
     * @return std::set<std::string> a set of person ids.
     */
    std::set<std::string> getDisappeared();

    /**
     * Initialize this manager with the given parameters to access TraCI.
     */
    void initialize(std::shared_ptr<TraCIConnection> connection, std::shared_ptr<TraCICommandInterface> commandInterface) override;

private:

    /**
     * This contains all updated persons. Will be cleared after
     * getUpdated() was called.
     */
    std::list<TraCIPerson> mUpdatedPersons;

    /**
     * This is the difference between mActivePersonIds and
     * currently active persons (provided with call to update()).
     *
     * This takes all the elements which are contained in
     * mActivePersons but not in currently active person ids.
     */
    std::set<std::string> mDisappearedPersons;

    /**
     * Get the active person id set.
     *
     * @return a set of string containing the active person ids.
     */
    std::set<std::string> getActivePersonIds();

    /**
     * Process a subscription result that contains an id list. This is
     * simply a helper method to improve clarity of update().
     */
    void processPersonIDList(std::list<std::string>& idList);

    /**
     * Subscribe to specific TraCI person variables.
     *
     * @param id the id of the person.
     */
    void subscribeToPersonVariables(std::string id);
};

} // end namespace TraCISubscriptionManagement
} // end namespace Veins

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_PERSONSUBSCRIPTIONMANAGER_H_ */
