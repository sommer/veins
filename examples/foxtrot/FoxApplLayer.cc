/* -*- mode:c++ -*- ********************************************************
 * file:        Foxtrot.cc
 *
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 ***************************************************************************
 * part of:     wsn-specific modules
 * description: aggregation layer: basic core. subclass to build your own
 *              aggregation protocol
 ***************************************************************************/


#include "FoxApplLayer.h"
#include "FoxtrotPacket.h"
#include "NetwControlInfo.h"
#include "AggPkt_m.h"
#include "winsupport.h"

#include <SinkAddress.h>

Define_Module_Like(FoxApplLayer, BaseApplLayer);

#define DBG(...) {char *dbg_out;asprintf(&dbg_out,## __VA_ARGS__);EV<<dbg_out;free(dbg_out);}
#define DBG_clear(...) {char *dbg_out;asprintf(&dbg_out,## __VA_ARGS__);EV_clear<<dbg_out;free(dbg_out);}

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we will set a timer to indicate the first time we will send a
 * message
 *
 **/
void FoxApplLayer::initialize(int stage)
{
	BaseApplLayer::initialize(stage);
	if (stage == 0)
	{
		isSink = getNode()->par("isSink").boolValue();
		EV << "myApplAddr = " << myApplAddr() << endl;
	}
	else if (stage == 1)
	{
		delayTimer = new cMessage("delay-timer", SEND_DATA);
		scheduleAt(simTime() + 0.005, delayTimer);
	}
}


/**
 * There are two kinds of messages that can arrive at this module: The
 * first (kind = DATA_MESSAGE) is a broadcast packet from a
 * neighbor node to which we have to send a reply. The second (kind =
 * BROADCAST_REPLY_MESSAGE) is a reply to a broadcast packet that we
 * have send and just causes some output before it is deleted
 **/
void FoxApplLayer::handleLowerMsg(cMessage * msg)
{
	AggPkt *m;
	switch (msg->kind())
	{
		case DATA_MESSAGE:
			m = check_and_cast < AggPkt * >(msg);
			if (isSink)
			{
				EV << "Received a aggregated packet from host[" << m->getSrcAddr() << "]" << endl;
				delete msg;
			}
			else
			{
				delete m->removeControlInfo();
				m->setControlInfo(new NetwControlInfo(SINK_ADDRESS));
				EV << "Forwarding sink packet from appl " << m->getSrcAddr() << endl;
				sendDown(m);
			}
			break;
		case 0xDEAD:
			{
				FoxtrotPacket *f = check_and_cast < FoxtrotPacket * >(msg);
				f->print("application");
				delete f;
				break;
			}
		default:
			DBG("Error! got packet with unknown kind: %x\n", msg->kind());
			if (msg->kind() == 0)
				error("blah");
			delete msg;
	}
}

/**
 * A timer with kind = SEND_BROADCAST_TIMER indicates that a new
 * broadcast has to be send (@ref sendData). 
 *
 * There are no other timer implemented for this module.
 *
 * @sa sendData
 **/
void FoxApplLayer::handleSelfMsg(cMessage * msg)
{
	switch (msg->kind())
	{
		case SEND_DATA:
			EV << "Sending aggregated packet" << endl;
			sendData();
			delete msg;
			delayTimer = NULL;
			break;
		default:
			EV << "Unknown selfmessage! -> delete, kind: " << msg->kind() << endl;
			delete msg;
	}
}

/**
 * This function creates a new broadcast message and sends it down to
 * the network layer
 **/
void FoxApplLayer::sendData()
{
	AggPkt *pkt = new AggPkt("DATA_MESSAGE", DATA_MESSAGE);
	pkt->setSrcAddr(myApplAddr());
	pkt->setDestAddr(SINK_ADDRESS);
	pkt->setLength(headerLength);
	pkt->setDataArraySize(1);
	pkt->setData(0, uniform(20, 21));	// FIXME: specify range in parameters

	EV << "Sending sink packet with data " << pkt->getData(0) << "\n";
	sendDown(pkt);
}

void FoxApplLayer::finish()
{
	BaseApplLayer::finish();
	cancelAndDelete(delayTimer);
	delayTimer = NULL;
}
