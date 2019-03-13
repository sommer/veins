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
#include "veins/modules/mobility/traci/subscriptionManagement/TrafficLight.h"

namespace veins {
namespace TraCISubscriptionManagement {

/**
 * @class TrafficLightSubscriptionManager
 *
 * This subscribes to specific TrafficLights (method: subscribe()) and
 * publishes the results of these subscriptions.
 *
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class TrafficLightSubscriptionManager : public SubscriptionManagerBase {

public:
    /**
     * Constructor.
     *
     * @param connection to the traci server to perform subscriptions.
     * @param commandInterface to the traci server to request variables.
     */
    TrafficLightSubscriptionManager(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface);

    /**
     * Default destructor.
     */
    virtual ~TrafficLightSubscriptionManager() = default;

    /**
     * Update this manager with the given buffer. The next call
     * to getTrafficLightUpdates() will change after a call to this.
     *
     * @param buffer the buffer containing the subscription information.
     *
     * @return true (default)
     */
    bool update(TraCIBuffer& buffer);

    /**
     * Subscribe to a specific traffic light.
     *
     * @param id of the traffic light to subscribe to.
     */
    void subscribe(std::string id);

    /**
     * Get the updates to all traffic lights since the last time
     * you called clearAPI()
     */
    std::vector<TrafficLight> getUpdated() const;

    /**
     * Clears the underlying container to the getUpdate() method
     * call.
     */
    void clearAPI() override;

private:
    /**
     * Stores the updates for traffic lights since the last time
     * getTrafficLightUpdates() got called.
     */
    std::vector<TrafficLight> updatedTrafficLights;
};

} // end namespace TraCISubscriptionManagement
} // namespace veins
