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

#include <utility>

#include "veins/modules/mobility/traci/subscriptionManagement/SubscriptionManagerBase.h"

namespace veins {
namespace TraCISubscriptionManagement {

SubscriptionManagerBase::SubscriptionManagerBase()
    : mSubscribedIds()
    , mConnection(nullptr)
    , mCommandInterface(nullptr)
{
}

void SubscriptionManagerBase::initialize(std::shared_ptr<TraCIConnection> connection, std::shared_ptr<TraCICommandInterface> commandInterface) {
    mConnection = connection;
    mCommandInterface = commandInterface;
}

bool SubscriptionManagerBase::isSubscribed(std::string id) {
    return mSubscribedIds.find(id) != mSubscribedIds.end();
}

bool SubscriptionManagerBase::addToSubscribed(std::string id) {
    auto result = mSubscribedIds.insert(id);
    return result.second;
}

bool SubscriptionManagerBase::removeFromSubscribed(std::string id) {
    bool result = isSubscribed(id);
    mSubscribedIds.erase(id);
    return result;
}

std::set<std::string> SubscriptionManagerBase::getSubscribed() {
    return mSubscribedIds;
}

std::shared_ptr<TraCIConnection> SubscriptionManagerBase::getConnection() {
    return mConnection;
}

std::shared_ptr<TraCICommandInterface> SubscriptionManagerBase::getCommandInterface() {
    return mCommandInterface;
}

} // end namespace TraCISubscriptionManagement
} /* namespace Veins */
