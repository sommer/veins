/* -*- mode:c++ -*- ********************************************************
 * file:        BurstApplLayer.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: application layer like the TestApplLayer but sends burst of
 *               messages
 **************************************************************************/


#include "BurstApplLayer.h"

#include "ApplPkt_m.h"

using std::endl;

Define_Module(BurstApplLayer);

// do some initialization
void BurstApplLayer::initialize(int stage)
{
    TestApplLayer::initialize(stage);

    if(stage==0){
        if(hasPar("burstSize"))
            burstSize = par("burstSize");
        else
            burstSize = 3;
        if(hasPar("burstReply"))
        	bSendReply = par("burstReply");
        else
        	bSendReply = true;
    }
}


void BurstApplLayer::handleSelfMsg(cMessage *msg)
{
    switch(msg->getKind())
    {
    case SEND_BROADCAST_TIMER:
        for(int i=0; i<burstSize; i++) {
            sendBroadcast();
        }
        break;
    default:
        EV <<" Unkown selfmessage! -> delete, kind: "<<msg->getKind()<<endl;
        break;
    }
}

/**
 * There are two kinds of messages that can arrive at this module: The
 * first (kind = BROADCAST_MESSAGE) is a broadcast packet from a
 * neighbor node to which we have to send a reply (if bSendReply is true). The second (kind =
 * BROADCAST_REPLY_MESSAGE) is a reply to a broadcast packet that we
 * have send and just causes some output before it is deleted
 */
void BurstApplLayer::handleLowerMsg( cMessage* msg )
{
	if ( !bSendReply && msg->getKind() == BROADCAST_MESSAGE) {
		ApplPkt *m  = static_cast<ApplPkt *>(msg);
		coreEV << "Received a broadcast packet from host["<<m->getSrcAddr()<<"] -> delete message, no reply" << endl;
		delete msg;
		return;
	}
	TestApplLayer::handleLowerMsg(msg);
}

