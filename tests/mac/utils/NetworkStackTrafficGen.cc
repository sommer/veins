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

#include "NetworkStackTrafficGen.h"
#include "MacControlInfo.h"
#include <cassert>
#include <Packet.h>


Define_Module(NetworkStackTrafficGen);

void NetworkStackTrafficGen::initialize(int stage)
{
	BaseLayer::initialize(stage);

	if(stage == 0) {
		world = FindModule<BaseWorldUtility*>::findGlobalModule();
		delayTimer = new cMessage("delay-timer", SEND_BROADCAST_TIMER);

		arp = BaseArpAccess().get();
		myNetwAddr = arp->myNetwAddr(this);

		packetLength = par("packetLength");
		packetTime = par("packetTime");
		pppt = par("packetsPerPacketTime");

		Packet p(1);
		catPacket = world->getCategory(&p);
	} else if (stage == 1) {
		scheduleAt(simTime() + dblrand() * 10, delayTimer);
	} else {

	}
}

void NetworkStackTrafficGen::finish()
{
}
void NetworkStackTrafficGen::handleSelfMsg(cMessage *msg)
{
	switch( msg->getKind() )
	{
	case SEND_BROADCAST_TIMER:
		assert(msg == delayTimer);
		sendBroadcast();
		ev << "scheduling new broadcast in " << packetTime*pppt << endl;
		scheduleAt(simTime() + packetTime/pppt, msg);
		break;
	default:
		EV << "Unkown selfmessage! -> delete, kind: "<<msg->getKind() <<endl;
		delete msg;
	}
}

// TODO implement
void NetworkStackTrafficGen::handleLowerMsg(cMessage *msg)
{
	Packet p(packetLength, 1, 0);
	world->publishBBItem(catPacket, &p, -1);

	delete msg;
	msg = 0;
}

// TODO implement
void NetworkStackTrafficGen::handleLowerControl(cMessage *msg)
{
	delete msg;
	msg = 0;
}

void NetworkStackTrafficGen::sendBroadcast()
{
	NetwPkt *pkt = new NetwPkt("BROADCAST_MESSAGE", BROADCAST_MESSAGE);
	pkt->setBitLength(packetLength);

	pkt->setSrcAddr(myNetwAddr);
	pkt->setDestAddr(L3BROADCAST);

	pkt->setControlInfo(new MacControlInfo(L2BROADCAST));

	Packet p(packetLength, 0, 1);
	world->publishBBItem(catPacket, &p, -1);

	sendDown(pkt);
}

