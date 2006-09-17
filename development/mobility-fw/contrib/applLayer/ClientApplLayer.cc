/***************************************************************************
 * file:        ClientApplLayer.cc
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
 * description: application layer: test client for the application layer
 *              in a centrally controlled network
 ***************************************************************************/


#include "ClientApplLayer.h"


Define_Module_Like(ClientApplLayer, BasicApplLayer);


/**
 * The only two messages that can arrive are BROADCAST_MESSAGE and
 * POLL_MESSAGE. In both cases just send the appropriate reply.
 **/
void ClientApplLayer::handleLowerMsg( cMessage *msg )
{
    ApplPkt *appl = static_cast<ApplPkt*>(msg);

  double delay;

  switch( msg->kind() ){

  case BROADCAST_MESSAGE:
    // delay message for some randomtime to simulate processing delay
    delay = uniform(0,0.003);

    EV <<" Received a broadcast ping from host[" << appl->getSrcAddr() 
       << "] -> send reply with delay "<<delay<<endl;

    appl->setDestAddr(appl->getSrcAddr());
    //address is the host modules' id()
    appl->setSrcAddr(myApplAddr());
    appl->setKind(BROADCAST_REPLY_MESSAGE);
    appl->setName( "broadcast-reply" );
    sendDelayedDown( appl, delay );
    break;

  case POLL_MESSAGE:
    EV <<" Received round-robin message from host["<<appl->getSrcAddr()<<"] send reply\n";

    appl->setDestAddr(appl->getSrcAddr());
    //address is the host modules' id()
    appl->setSrcAddr(myApplAddr());
    appl->setKind(POLL_REPLY);
    appl->setName( "Poll-reply" );
    sendDown( appl );
    break;

  default:
    EV <<"Error! got packet with unknown kind: " << msg->kind()<<endl;
    delete msg;
  }
}

