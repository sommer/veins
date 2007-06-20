#include "TestLocalization.h"
#include "NetwControlInfo.h"

#include <SimpleAddress.h>
#include <assert.h>

Define_Module_Like(TestLocalization, BaseLocalization);

void TestLocalization::initialize(int stage)
{
	BaseLocalization::initialize(stage);
	if (stage == 0) {
		delayTimer = new cMessage("delay-timer", SEND_BROADCAST_TIMER);
		for (int i = 0; i < MAX_NEIGHBOURS; i++)
			neighbours[i] = false;

	} else if (stage == 1) {
		scheduleAt(simTime() + findHost()->index() + 0.005, delayTimer);
	}
}

void TestLocalization::handleLowerMsg(cMessage * msg)
{
	switch (msg->kind()) {
	case LOCALIZATION_MSG: {
		LocPkt *m = static_cast < LocPkt * >(msg);
		
		switch (m->getType()) {
		case BROADCAST_MESSAGE:
			EV << "Received a broadcast packet from host[" << m->getSrcAddr() << "] -> sending reply\n";
			sendReply(m);
			break;
		case BROADCAST_REPLY_MESSAGE:
			EV << "Received reply from host[" << m->getSrcAddr() << "]; delete msg\n";
			delete msg;
			break;
		default:
			EV << "Error! got localization packet with unknown type: " << m->getType() << endl;
			delete msg;
		}
	}
		break;
	default:
		EV << "Error! got packet with unknown kind: " << msg->kind() << endl;
		delete msg;
		assert (false);
	}
}

void TestLocalization::handleSelfMsg(cMessage * msg)
{
	switch (msg->kind()) {
	case SEND_BROADCAST_TIMER:
		sendBroadcast();
		delete msg;
		break;
	default:
		EV << "Unkown selfmessage! -> delete, kind: " << msg->kind() << endl;
		delete msg;
	}
}

void TestLocalization::sendBroadcast()
{
	LocPkt *pkt = new LocPkt("LOCALIZATION_MSG", LOCALIZATION_MSG);

	pkt->setDestAddr(-1);
	pkt->setSrcAddr(myApplAddr());
	pkt->setType(BROADCAST_MESSAGE);
	pkt->setName("BROADCAST_MESSAGE");
	pkt->setLength(headerLength);

	// set the control info to tell the network layer the layer 3 address;
	pkt->setControlInfo(new NetwControlInfo(L3BROADCAST));

	EV << "Sending broadcast packet!\n";
	sendDown(pkt);
}

void TestLocalization::sendReply(LocPkt * msg)
{
	double delay;

	if (neighbours[msg->getSrcAddr()] == false) {
		neighbours[msg->getSrcAddr()] = true;
		EV << "neighbours[" << myApplAddr() << "]: ";
		for (int i = 0; i < MAX_NEIGHBOURS; i++) {
			if (neighbours[i])
				ev << i << "; ";
		}
		ev << endl;
	}

	delay = uniform(0, 0.01);

	msg->setDestAddr(msg->getSrcAddr());
	msg->setSrcAddr(myApplAddr());
	msg->setType(BROADCAST_REPLY_MESSAGE);
	msg->setName("BROADCAST_REPLY_MESSAGE");

	//NOTE: the NetwControl info was already set by the network layer and stays the same
	sendDelayedDown(msg, delay);

	EV << "sent message with delay " << delay << endl;
}



void TestLocalization::finish()
{
	BaseLocalization::finish();
	if (!delayTimer->isScheduled())
		delete delayTimer;
}
