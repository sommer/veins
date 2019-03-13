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

#include <string>
#include <vector>

#include "veins/veins.h"

#include "veins/modules/mobility/traci/subscriptionManagement/SubscriptionManagerBase.h"
#include "veins/modules/mobility/traci/subscriptionManagement/Person.h"

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
 * @author Nico Dassler <dassler@hm.edu>
 */
class PersonSubscriptionManager : public SubscriptionManagerBase {

public:
    /**
     * Constructor.
     *
     * @param connection to the traci server to perform subscriptions.
     * @param commandInterface to the traci server to request variables.
     */
    PersonSubscriptionManager(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface);

    /**
     * Default destructor.
     */
    virtual ~PersonSubscriptionManager() = default;

    /**
     * Perform an explicit update of this manager.
     *
     * If you dont receive id_list subscription you can force updates of the internal
     * manager state using this function. This essentially does the same as the
     * update() method when the buffer supplied to it contains an id list subscription.
     *
     * This method takes the currently active person ids supplied with the call to this
     * method and compares it with the current subscribed persons. Based on that list
     * it will calculate which persons disappeared and appeared.
     *
     * Note: The param is a list because that is what is supplied when using the command
     * interface.
     *
     * @param currentlyActivePersonIds A list containing the currently active persons
     * identified by their ids.
     *
     */
    void updateWithList(std::list<std::string>& currentlyActivePersonIds);

    bool update(TraCIBuffer& buffer) override;

    /**
     * This gives you all the persons that were updated (includes
     * new persons).
     *
     * This will change when update(), updateWithList() is
     * called. Will return nothing immediately after clearAPI was called (
     * until next update() call).
     *
     * @return a list of TraCIPerson.
     */
    std::vector<Person> getUpdated() const;

    /**
     * This gives you all the disappeared persons.
     *
     * This will change when update(), updateWithList() is called. Will return
     * nothing immediately after clearAPI() was called.
     *
     * @return std::vector<std::string> a set of person ids.
     */
    std::vector<std::string> getDisappeared() const;

    /**
     * Clear the API.
     *
     * In this case it will clear class variables that will
     * affect calls to:
     *  - getUpdated()
     *  - getDisappeared()
     *
     */
    void clearAPI() override;

private:
    /**
     * This contains all updated persons. Will be cleared after
     * clearAPI() was called.
     */
    std::vector<Person> updatedPersons;

    /**
     * This contains all disappeared persons. Will be cleared after
     * clearAPI() was called.
     */
    std::set<std::string> disappearedPersons;

    /**
     * Process a subscription result that contains an id list. This is
     * simply a helper method to improve clarity of update().
     */
    void processPersonIDList(std::list<std::string>& idList);

    /**
     * Performs an initial subscription to person id list and evaluates the
     * response.
     */
    void subscribeToPersonIDList();

    /**
     * Subscribe to specific TraCI person variables.
     *
     * The variables are:
     *  - TraCIConstants::VAR_POSITION
     *  - TraCIConstants::VAR_ROAD_ID
     *  - TraCIConstants::VAR_SPEED
     *  - TraCIConstants::VAR_ANGLE
     *  - TraCIConstants::VAR_TYPE;
     *
     * @param id the id of the person.
     */
    void subscribeToPersonVariables(std::string id);
};

} // end namespace TraCISubscriptionManagement
} // namespace veins
