//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "veins_inet/VeinsInetMobility.h"
#include "veins/modules/utility/TimerManager.h"

namespace Veins {

class VeinsInetApplicationBase : public inet::ApplicationBase, public inet::UdpSocket::ICallback {
protected:
    Veins::VeinsInetMobility* mobility;
    Veins::TraCICommandInterface* traci;
    Veins::TraCICommandInterface::Vehicle* traciVehicle;
    Veins::TimerManager timerManager{this};

    inet::L3Address destAddress;
    const int portNumber = 9001;
    inet::UdpSocket socket;

protected:
    virtual int numInitStages() const override;
    virtual void initialize(int stage) override;
    virtual bool handleNodeStart(inet::IDoneCallback* doneCallback) override;
    virtual bool startApplication();
    virtual bool stopApplication();
    virtual bool handleNodeShutdown(inet::IDoneCallback* doneCallback) override;
    virtual void handleNodeCrash() override;
    virtual void finish() override;

    virtual void refreshDisplay() const override;
    virtual void handleMessageWhenUp(inet::cMessage* msg) override;

    virtual void socketDataArrived(inet::UdpSocket* socket, inet::Packet* packet) override;
    virtual void socketErrorArrived(inet::UdpSocket* socket, inet::Indication* indication) override;

    virtual std::unique_ptr<inet::Packet> createPacket(std::string name);
    virtual void processPacket(std::shared_ptr<inet::Packet> pk);
    virtual void timestampPayload(inet::Ptr<inet::Chunk> payload);
    virtual void sendPacket(std::unique_ptr<inet::Packet> pk);

public:
    VeinsInetApplicationBase();
    ~VeinsInetApplicationBase();
};

} // namespace Veins
