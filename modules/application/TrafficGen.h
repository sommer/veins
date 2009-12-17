//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef TRAFFICGEN_H_
#define TRAFFICGEN_H_

#include "BaseApplLayer.h"
#include <BaseWorldUtility.h>

#include <omnetpp.h>

/**
 * @brief A module to generate a certain rate of traffic.
 */
class TrafficGen : public BaseApplLayer
{
public:

	enum TrafficGenMessageKinds{
		SEND_PACKET_TIMER = LAST_BASE_APPL_MESSAGE_KIND,
		TRAFFIC_GEN_PACKET,
		LAST_TRAFFIC_GEN_MESSAGE_KIND
	};

protected:

	simtime_t packetTime;
	double pppt;
	int burstSize;
	int remainingBurst;

	int catPacket;

	long nbPacketDropped;

	cMessage *delayTimer;

	BaseWorldUtility* world;

public:


protected:

	virtual void initialize(int stage);

	virtual void finish();

	/** @brief Handle self messages such as timer... */
	virtual void handleSelfMsg(cMessage *msg);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerMsg(cMessage *msg);

	/** @brief Send a broadcast message to lower layer. */
	virtual void sendBroadcast();
};

#endif
