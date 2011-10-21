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

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseApplLayer.h"

class BaseWorldUtility;

/**
 * @brief A module to generate a certain rate of traffic.
 */
class MIXIM_API TrafficGen : public BaseApplLayer
{
public:

	/** @brief The message kinds used by this layer.*/
	enum TrafficGenMessageKinds {
		/** @brief Schedules sending of a new message.*/
		SEND_PACKET_TIMER = LAST_BASE_APPL_MESSAGE_KIND,
		/** @brief The kind for a packet send by this layer.*/
		TRAFFIC_GEN_PACKET,
		/** @brief Sub classing layers shoudl begin their own kinds at this value.*/
		LAST_TRAFFIC_GEN_MESSAGE_KIND
	};

protected:

	/** @brief The time it takes to transmit a packet.
	 * Bit length divided by bitrate.*/
	simtime_t packetTime;

	/** @brief Packets per packet time. More or less the load
	 * this layer should generate.*/
	double pppt;

	/** @brief The number of packets to send at once.*/
	int burstSize;

	/** @brief The remainign apckets to send for the current burst.*/
	int remainingBurst;

	/** @brief Tracks the number of packets dropped so far.*/
	long nbPacketDropped;

	/** @brief Timer message to schedule next packet send.*/
	cMessage *delayTimer;

	/** @brief Pointer to world utility module.*/
	BaseWorldUtility* world;

public:
	virtual ~TrafficGen();
	/** @brief Omnet++ Initialisation.*/
	virtual void initialize(int stage);

	/** @brief Called at the end of the simulation to record statistics.*/
	virtual void finish();

protected:

	/** @brief Handle self messages such as timer... */
	virtual void handleSelfMsg(cMessage *msg);

	/** @brief Handle messages from lower layer */
	virtual void handleLowerMsg(cMessage *msg);

	/** @brief Send a broadcast message to lower layer. */
	virtual void sendBroadcast();
};

#endif
