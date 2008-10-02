#include "DetailedPhy.h"
#include "NicControlType.h"

#include <assert.h>

//Define_Module_Like(DetailedPhy, BasePhyLayer);

void DetailedPhy::initialize(int stage)
{
	CollisionsPhy::initialize(stage);
	if (stage == 0)
		state = LISTEN;
}

void DetailedPhy::handleLowerMsgStart(cMessage* msg)
{
	assert(msg->getFullPath().find("TimerCore")==std::string::npos);
	CollisionsPhy::handleLowerMsgStart(msg);
	if (!colliding)
	{
		sendControlUp(new cMessage("RX_START",NicControlType::RX_START));

		/* FIXME: cheating! This should be sent a bit later */
		cMessage *dup = (cMessage*)msg->getEncapsulatedMsg()->dup();
		dup->setKind(NicControlType::RX_HDR);
		sendControlUp(dup);
	}
}

void DetailedPhy::handleLowerMsgEnd(cMessage* msg)
{
	CollisionsPhy::handleLowerMsgEnd(msg);
}

void DetailedPhy::handleUpperMsg(cMessage* msg)
{
	if (state != TRANSMIT)
		error("need to be in transmit before we will send messages");
	CollisionsPhy::handleUpperMsg(msg);
}

void DetailedPhy::handleCollision(cMessage *msg)
{
	sendControlUp(new cMessage("RX_FAIL",NicControlType::RX_FAIL));
}

void DetailedPhy::handleUpperControl(cMessage* msg)
{
	switch(msg->getKind())
	{
		case NicControlType::SET_TRANSMIT:
			state = TRANSMIT;
			if (messages>0)
				colliding = true;
			coreEV <<"detailed phy transmit"<<endl;
			break;

		case NicControlType::SET_LISTEN:
			coreEV <<"detailed phy listen"<<endl;
			if (messages == 0)
				colliding = false;
			state = LISTEN;
			break;

		case NicControlType::SET_SLEEP:
			coreEV <<"detailed phy sleep"<<endl;
			state = SLEEP;
			break;
			
		default:
			CollisionsPhy::handleUpperControl(msg);
			return;
	}
	delete msg;
}

void DetailedPhy::increment()
{
	CollisionsPhy::increment();
	if (messages>0 && state == TRANSMIT)
		colliding = true;
	//coreEV << "increment to "<<messages<<endl;
}

void DetailedPhy::decrement()
{
	bool wascollide = colliding;
	CollisionsPhy::decrement();
	//coreEV << "decrement to "<<messages<<endl;
	if (wascollide && state == TRANSMIT)
		colliding = true;
}
