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
#include <set>
#include <memory>

#include "veins/veins.h"

#include "veins/modules/mobility/traci/TraCIBuffer.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConnection.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class SubscriptionManagerBase
 *
 * This is an abstract class providing common for subscription managers. Each subscription
 * manager must have an update() method to inform it about subscription updates. To create
 * new subscriptions this class will provide a command interface and the connection to the
 * TraCI server. It will also provide an interface to store which ids have aready been
 * subscribed.
 *
 * A subscription manager is called via the update() method to update its internal state. For
 * example which vehicles are appeared/disappeared in/from the simulation. These state
 * changes can then be requested via the managers api (these are methods which are defined
 * additionally in the child classes and vary depending on what the subscription manager is
 * supposed to do).
 *
 * @author Nico Daßler <dassler@hm.edu>
 */
class SubscriptionManagerBase {

public:
    /**
     * Constructor.
     *
     * @param connection to the traci server to perform subscriptions.
     * @param commandInterface to the traci server to request variables.
     */
    SubscriptionManagerBase(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface);

    /**
     * Default destructor.
     */
    virtual ~SubscriptionManagerBase() = default;

    /**
     * Udpate the manager with the given buffer (subscription response).
     *
     * Each subscription manager has to implement this method. The given buffer
     * is expected to start with the response object id. See the TraCI documentation
     * for more details:
     * https://sumo.dlr.de/wiki/TraCI/Object_Variable_Subscription#Response_0xeX:_..._Subscription_Response
     *
     * The method should read one complete subscription response from the buffer. Nothing
     * more and nothing less. See the documentation linked above.
     *
     * When the manager is updated you will receive updated responses when
     * calling the other methods of this class.
     *
     *
     * @param buffer the TraCIBuffer containing the subscription response.
     *
     * @return if there was an id_list was received in the buffer (not all subscription manager will use
     * this. the result is undefined for them).
     */
    virtual bool update(TraCIBuffer& buffer) = 0;

    /**
     * Clears the underlying containers which are made public by the API. Has
     * to be implemented by each manager.
     */
    virtual void clearAPI() = 0;

protected:

    /**
     * Checks if this id is subscribed to by this manager.
     *
     * @param id to check.
     *
     * @return true if the given id is subscribed. False if not.
     */
    bool isSubscribed(std::string id);

    /**
     * Add the given id to subscribed ids set.
     *
     * @param id to subscribe to.
     *
     */
    void addToSubscribed(std::string id);

    /**
     * Removes the give id from subscribed ids set.
     *
     * @param id to remove.
     */
    void removeFromSubscribed(std::string id);

    /**
     * Returns a copy of subscribed ids.
     *
     * @return set of subscribed ids.
     */
    std::vector<std::string> getSubscribed() const;

    /**
     * Get the pointer to connection to access TraCI server.
     *
     * @return shared pointer to TraCIConnection.
     */
    std::shared_ptr<TraCIConnection> getConnection() const;

    /**
     * Get the pointer to command interface to access TraCI server.
     *
     * @return shared pointer to TraCICommandInterface.
     */
    std::shared_ptr<TraCICommandInterface> getCommandInterface() const;

private:
    /**
     * Stores the ids that this manager has subscribed to.
     */
    std::set<std::string> subscribedIds;

    /**
     * Stores the connection to TraCI server.
     */
    std::shared_ptr<TraCIConnection> connection;

    /**
     * Stores the command interface to TraCI server.
     */
    std::shared_ptr<TraCICommandInterface> commandInterface;
};

} // end namespace TraCISubscriptionManagement
} // namespace veins
