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

#include <utility>

#include "veins/modules/mobility/traci/subscriptionManagement/SubscriptionManagerBase.h"

namespace veins {
namespace TraCISubscriptionManagement {

SubscriptionManagerBase::SubscriptionManagerBase(std::shared_ptr<TraCIConnection>& connection, std::shared_ptr<TraCICommandInterface>& commandInterface)
    : subscribedIds()
    , connection(connection)
    , commandInterface(commandInterface)
{
}

bool SubscriptionManagerBase::isSubscribed(std::string id)
{
    return subscribedIds.find(id) != subscribedIds.end();
}

void SubscriptionManagerBase::addToSubscribed(std::string id)
{
    subscribedIds.insert(id);
}

void SubscriptionManagerBase::removeFromSubscribed(std::string id)
{
    subscribedIds.erase(id);
}

std::vector<std::string> SubscriptionManagerBase::getSubscribed() const
{
    std::vector<std::string> output;
    std::copy(subscribedIds.begin(), subscribedIds.end(), std::back_inserter(output));
    return output;
}

std::shared_ptr<TraCIConnection> SubscriptionManagerBase::getConnection() const
{
    return connection;
}

std::shared_ptr<TraCICommandInterface> SubscriptionManagerBase::getCommandInterface() const
{
    return commandInterface;
}

} // end namespace TraCISubscriptionManagement
} // namespace veins
