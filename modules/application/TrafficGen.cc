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

#include "TrafficGen.h"
#include "NetwControlInfo.h"
#include <cassert>
#include <Packet.h>
#include <BaseNetwLayer.h>


Define_Module(TrafficGen);

void TrafficGen::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	if(stage == 0) {
		world = FindModule<BaseWorldUtility*>::findGlobalModule();
		delayTimer = new cMessage("delay-timer", SEND_PACKET_TIMER);

		packetTime = par("packetTime");
		pppt = par("packetsPerPacketTime");
		burstSize = par("burstSize");

		nbPacketDropped = 0;

		Packet p(1);
		catPacket = world->getCategory(&p);
	} else if (stage == 1) {
		if(burstSize > 0) {
			remainingBurst = burstSize;
			scheduleAt(dblrand() * packetTime * burstSize / pppt, delayTimer);
		}
	} else {

	}
}

TrafficGen::~TrafficGen() {
	cancelAndDelete(delayTimer);
}

void TrafficGen::finish()
{
	recordScalar("dropped", nbPacketDropped);
}

void TrafficGen::handleSelfMsg(cMessage *msg)
{
	switch( msg->getKind() )
	{
	case SEND_PACKET_TIMER:
		assert(msg == delayTimer);


		sendBroadcast();

		remainingBurst--;

		if(remainingBurst == 0) {
			remainingBurst = burstSize;
			scheduleAt(simTime() + (dblrand()*1.4+0.3)*packetTime * burstSize / pppt, msg);
		} else {
			scheduleAt(simTime() + packetTime * 2, msg);
		}

		break;
	default:
		EV << "Unkown selfmessage! -> delete, kind: "<<msg->getKind() <<endl;
		delete msg;
	}
}


void TrafficGen::handleLowerMsg(cMessage *msg)
{
	cPacket* pkt = static_cast<cPacket*>(msg);
	Packet p(pkt->getBitLength(), 1, 0);
	world->publishBBItem(catPacket, &p, -1);

	delete msg;
	msg = 0;
}


void TrafficGen::sendBroadcast()
{
	ApplPkt *pkt = new ApplPkt("BROADCAST_MESSAGE", TRAFFIC_GEN_PACKET);
	pkt->setDestAddr(-1);
	// we use the host modules getIndex() as a appl address
	pkt->setSrcAddr( myApplAddr() );
	pkt->setBitLength(headerLength);

	// set the control info to tell the network layer the layer 3
	// address;
	pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );

	EV << "Sending broadcast packet!\n";
	sendDown( pkt );
}

