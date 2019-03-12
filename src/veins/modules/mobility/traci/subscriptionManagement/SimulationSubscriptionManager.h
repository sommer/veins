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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_SIMULATIONSUBSCRIPTIONMANAGER_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_SIMULATIONSUBSCRIPTIONMANAGER_H_

#include <list>

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
 * IMPORTANT: Call initialize() before using!
 *
 * @author Nico Dassler <dassler@hm.edu>
 */
class SimulationSubscriptionManager: public SubscriptionManagerBase {
public:

    /**
     * Constructor.
     */
    SimulationSubscriptionManager();

    /**
     * Default destructor.
     */
    virtual ~SimulationSubscriptionManager() = default;

    /**
     * Initialize this manager.
     *
     * The manager gets initialized with the given variables and a subscription
     * to simulation values is performed.
     *
     * @param connection shared ptr to connection to traci server.
     * @param commmandInterface shared pointer to command interface to traci server.
     */
    void initialize(std::shared_ptr<TraCIConnection> connection, std::shared_ptr<TraCICommandInterface> commandInterface) override;

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

private:

    /**
     * Stores the ids that started teleporting since the last time
     * getStartedTeleporting() was called.
     */
    std::list<std::string> mStartedTeleporting;

    /**
     * Stores the ids that started parking since the last time
     * getStartedParking() was called.
     */
    std::list<std::string> mStartedParking;

    /**
     * Stores the ids that ended Parking since the last time
     * getEndedParking() was called.
     */
    std::list<std::string> mEndedParking;
};

} // end namespace TraCISubscriptionManagement
} /* namespace Veins */

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_SUBSCRIPTIONMANAGEMENT_SIMULATIONSUBSCRIPTIONMANAGER_H_ */
