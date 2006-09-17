/* -*- mode:c++ -*- ********************************************************
 * file:        PollApplLayer.cc
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
 * description: application layer for a simple base station, which polls all 
 *              clients in a round-robin manner
 **************************************************************************/


#include "PollApplLayer.h"
#include "NetwControlInfo.h"

#include <SimpleAddress.h>

Define_Module(PollApplLayer);

/**
 * Read ned parameters and schedule first broadcast timer
 **/
void PollApplLayer::initialize(int stage)
{
  ClientApplLayer::initialize(stage);

  if(stage==0){
    numClients = par("numClients");
    broadcastInterval = par("broadcastInterval");
    pollTimeout = par("pollTimeout");
    aliveClients = 0;

    pollTimer = new cMessage("poll", POLL_REPLY_TIMEOUT);
  }
  else if(stage==1){
    //send an initial broadcast timer
    scheduleAt(simTime() + 0.005, new cMessage("send-broadcast", SEND_BROADCAST_TIMER));
  }
}

void PollApplLayer::finish()
{
    if( !pollTimer->isScheduled() ) 
	delete pollTimer;
}

/**
 * There are two kinds of messages that can arrive at this module: The
 * first (kind = BROADCAST_REPLY_MESSAGE) is a reply to a broadcast
 * packet that we have sent and results in an update of the @ref
 * clients list. The second (kind = POLL_REPLY) is a reply to a poll
 * packet that we have sent. The @ref pollTimer has to be deleted and
 * the next client can be polled.
 **/
void PollApplLayer::handleLowerMsg( cMessage *msg )
{
    ApplPkt *appl = static_cast<ApplPkt*>(msg);
    NetwControlInfo* cInfo;

  switch( msg->kind() ){

  case BROADCAST_REPLY_MESSAGE:
    EV <<"Received broadcast reply from host["<<appl->getSrcAddr()
       <<"] update client list.\n";

    cInfo = dynamic_cast<NetwControlInfo*>(msg->removeControlInfo());

    clients[appl->getSrcAddr()]=true;
    clientAddrs[appl->getSrcAddr()]= cInfo->getNetwAddr();
    aliveClients++;
    delete msg;
    delete cInfo;
    break;

  case POLL_REPLY:
    EV <<"Received reply from host["<<appl->getSrcAddr()<<"] poll next client\n";
    cancelEvent( pollTimer );
    pollNext();
    delete msg;
    break;

  default:
    EV <<"Error! got packet with unknown kind: " << msg->kind()<<endl;
    delete msg;
  }
}

/**
 * A timer with kind = SEND_BROADCAST_TIMER indicates that a new
 * broadcast has to be send (@ref sendBroadcast). 
 *
 * kind = POLL_REPLY_TIMEOUT invalidates the client entry, and for now
 * just prints debug output and calls @ref pollNext to poll the next
 * client in the list.
 *
 * kind = BROADCAST_REPLY_TIMEOUT indicates the end of the broadcast
 * period and starts the polling period.
 *
 * @sa sendBroadcast, pollNext
 **/
void PollApplLayer::handleSelfMsg(cMessage *msg)
{
  switch(msg->kind()){

  case SEND_BROADCAST_TIMER:
    //invalidate all client entries
    clients.clear();
    clientAddrs.clear();
    aliveClients = 0;
    sendBroadcast();
    //reschedule broadcast timer
    scheduleAt(simTime() + broadcastInterval, msg);
    //schedule reply timeout
    scheduleAt(simTime() + 3*pollTimeout, new cMessage("bk-reply", BROADCAST_REPLY_TIMEOUT));
    break;

  case POLL_REPLY_TIMEOUT:
      EV <<"Client "<<getLogName(clientAddrs[(*it).first])<<" does not answer. Poll next\n";
      (*it).second = false;
      pollNext();
      break;

  case BROADCAST_REPLY_TIMEOUT:
    EV <<"Broadcast period over, "<<aliveClients<<" of "<<numClients<<" reachable; start polling\n";

    // do nothing if list is empty
    if( !clients.empty() ){

	//set iterator to beginning of the map
	it=clients.begin();
	
	//start polling
	//only send poll message if client is reachable
	if( (*it).second ){
	    EV <<"poll first client ("<<getLogName(clientAddrs[(*it).first])<<") with address "<<clientAddrs[(*it).first]<<endl;
	    sendPoll( (*it).first );
	}
	else{
	    EV <<"client "<<getLogName(clientAddrs[(*it).first])<<" not reachable, poll next\n";
	    
	    pollNext();
	}
    }

    delete msg;
    break;

  default:
    EV <<"Unkown selfmessage! -> delete, kind: "<<msg->kind()<<endl;
    delete msg;
  }
}

/**
 * This function creates a new broadcast message and sends it down to
 * the network layer
 **/
void PollApplLayer::sendBroadcast()
{
    ApplPkt *pkt = new ApplPkt( "broadcast" );
    pkt->setDestAddr(-1);
    // we use the host modules id() as a appl address
    pkt->setSrcAddr(myApplAddr());
    pkt->setLength(headerLength);
    pkt->setKind(BROADCAST_MESSAGE);

    // set the control info to tell the network layer the layer 3
    // address;
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    
    EV <<"Sending broadcast packet!\n";
    sendDown( pkt );
}

/**
 * This function creates a new poll message to poll 'addr' and sends
 * it down to the network layer
 *
 * @param addr Address of the client to poll.
 **/
void PollApplLayer::sendPoll( int addr )
{
    ApplPkt *pkt = new ApplPkt( "Poll" );
    pkt->setDestAddr(addr);
    // we use the host modules id() as a appl address
    pkt->setSrcAddr(myApplAddr());
    pkt->setLength(headerLength);
    pkt->setKind(POLL_MESSAGE);
    
    // set the control info to tell the network layer the layer 3
    // address;
    pkt->setControlInfo( new NetwControlInfo(clientAddrs[addr]) );
    
    EV <<"Sending poll packet to "<<getLogName(clientAddrs[(*it).first])<<" with address "<<clientAddrs[addr]<<endl;
    sendDown( pkt );
    
    //schedule timer
    scheduleAt( simTime() + pollTimeout, pollTimer );
}

/**
 * Poll the next reachable client in the list. If there are no more
 * clients, just return.
 **/
void PollApplLayer::pollNext()
{
    //increase iterator
    ++it;

  if( it==clients.end() ){
    EV <<"no more clients to poll\n";
    return;
  }

  //only send poll message if client is reachable
  if( (*it).second ){
    EV <<"poll next client ("<<getLogName(clientAddrs[(*it).first])<<") with address "<<clientAddrs[(*it).first]<<endl;
    sendPoll( (*it).first );
  }
  else{
    EV <<"client "<<getLogName(clientAddrs[(*it).first])<<" not reachable, poll next\n";

    pollNext();
  }
}
