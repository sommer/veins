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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_SUBSCRIPTIONMANAGERBASE_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_SUBSCRIPTIONMANAGERBASE_H_

#include <string>
#include <set>
#include <memory>

#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"


namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class SubscriptionManagerBase
 *
 * This class provides some common functionality for a subscription manager. It stores
 * TraCIConnection, TraCICommandInterface and a set containing subscribed ids.
 *
 * It also enforces you to provide an update() method in your subscription manager.
 *
 * @author Nico Daßler <dassler@hm.edu>
 */
class SubscriptionManagerBase {

public:
    /**
     * Constructor.
     */
    SubscriptionManagerBase();

    /**
     * Default destructor.
     */
    virtual ~SubscriptionManagerBase() = default;

    /**
     * Each subscription manager has to implement this method. The given buffer
     * is assumed to have the first few bytes removed. These bytes represent
     * the first few fields of an answer to a simulation step request or the
     * request to subscribe variable request.
     * The fields that should be removed are:
     *      - the number of subscription results
     *      - the response command length
     *      - the response command length extended
     *      - the id of the response commmand
     *
     * This method is expected to change the results to api methods of this manager.
     *
     * @param buffer the TraCIBuffer containing the subscription response.
     *
     * @return if there was an id_list was received in the buffer (not all subscription manager will use
     * this. the result is undefined for them).
     */
    virtual bool update(TraCIBuffer& buffer) = 0;

    /**
     * This initializes this manager with the necessary connection and command interface
     * to perform requests to the TraCI server.
     *
     * Important: Always do this before using a subscription manager.
     *
     * Note: When overriding the this method from a child class make sure to call the base
     * class implementation to set mConnection and mCommandInterface variables.
     *
     * @param connection to access traci server (for example: to perform subscriptions).
     * @param commandInterface to access traci (for example: direct request to get vehicle id_list)
     */
    virtual void initialize(std::shared_ptr<TraCIConnection> connection, std::shared_ptr<TraCICommandInterface> commandInterface);

    /**
     * Checks if this id is subscribed to by this manager.
     *
     * @param id to check.
     *
     * @return true if the given id is subscribed. False if not.
     */
    bool isSubscribed(std::string id);

    /**
     * Subscribe to the given id.
     *
     * @param id to subscribe to.
     *
     * @return true if adding was successful.
     */
    bool addToSubscribed(std::string id);

    /**
     * Removes the give id from subscribed.
     *
     * @param id to remove.
     *
     * @return true if removing successful.
     */
    bool removeFromSubscribed(std::string id);

    /**
     * Returns a copy of subscribed ids.
     *
     * @return set of subscribed ids.
     */
    std::set<std::string> getSubscribed();

    /**
     * Get the pointer to connection to access TraCI server.
     *
     * @return shared pointer to TraCIConnection.
     */
    std::shared_ptr<TraCIConnection> getConnection();

    /**
     * Get the pointer to command interface to access TraCI server.
     *
     * @return shared pointer to TraCICommandInterface.
     */
    std::shared_ptr<TraCICommandInterface> getCommandInterface();

private:

    /**
     * Stores the ids that this manager has subscribed to.
     */
    std::set<std::string> mSubscribedIds;

    /**
     * Stores the connection to TraCI server.
     */
    std::shared_ptr<TraCIConnection> mConnection;

    /**
     * Stores the command interface to TraCI server.
     */
    std::shared_ptr<TraCICommandInterface> mCommandInterface;
};

} // end namespace TraCISubscriptionManagement
} /* namespace Veins */

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_SUBSCRIPTIONMANAGERBASE_H_ */
