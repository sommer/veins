/* -*- mode:c++ -*- ********************************************************
 * file:        Centroid.cc
 *
 * author:      Aline Baggio
 *
 * copyright:   (C) 2007 Parallel and Distributed Systems Group (PDS) at
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
 * description: centroid class for the localization module
 **************************************************************************/

/* ************************************************************************
 * This file is an adapted copy of locNetwork/TestLocAppl and 
 * implements a Centroid localization algorithm (with moving anchors, 
 * see omnetpp.ini file)
 **************************************************************************/

#include "centroid.h"
#include "NetwControlInfo.h"
#include <SimpleAddress.h>
//#include <BaseMobility.h>
#include <ConstSpeedMobility.h>

Define_Module_Like(Centroid, BaseLocalization);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we will set a timer to indicate the first time we will send a
 * message
 *
 **/
void Centroid::initialize(int stage)
{
    BaseLocalization::initialize(stage);
    if(stage == 0) {
      Timer::init(this);

      EV <<"initialize - Stage 0, id: "<<id<<"\n";
      EV <<"initialize - Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
      if(isAnchor) { /* Node is an anchor */
	EV << "\t\tInitializing centroid application for anchor "<<id<<"...\n";
	setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
      }
      else{ /* Node is a regular node */
	EV << "\t\tInitializing centroid application for node "<<id<<"...\n";
	setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);
      }
    }
}


/**
 * There are two kinds of messages that can arrive at this module: The
 * first (kind = ANCHOR_BROADCAST_MESSAGE) is a broadcast packet from a
 * neighbor node to which we have to send a reply. The second (kind =
 * NODE_POSITION_MESSAGE (NODE_BROADCAST_MESSAGE in centroid.h)) is a reply to a broadcast packet that we
 * have send and just causes some output before it is deleted
 **/
//void Centroid::handleLowerMsg( cMessage* msg )
void Centroid::handleMsg( cMessage* msg )
{
    LocPkt *m;
    simtime_t receivedTs;
    int receivedId;
    double x, y, z;
    Location loc;
    bool receivedIsAnchor;

    EV <<"handleMsg - Node: "<<id<<" Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
    m = dynamic_cast < LocPkt * >(msg);

    receivedId=m->getId();
    loc=m->getPos();
    x=loc.getX();
    y=loc.getY();
    z=loc.getZ();
    receivedTs=loc.getTimestamp();
    receivedIsAnchor=m->getIsAnchor();

    switch(m->kind()){
    case ANCHOR_BROADCAST_MESSAGE:
	EV << "Received a broadcast packet from anchor "<<m->getId()<<"\n";
	if(isAnchor){/* anchor only stuff */
	  EV << "Anchor "<<id<<" got anchor position message with contents: "<<receivedId<<"("<<x<<","<<y<<","<<z<<") at "<<receivedTs<<" anchor:"<<receivedIsAnchor<<"\n";
	}
	delete msg;
        break;
    case NODE_BROADCAST_MESSAGE:
	EV << "Node/anchor "<<id<<" got node position message with contents: "<<m->getId()<<"("<<m->getPos().getX()<<","<<m->getPos().getY()<<","<<m->getPos().getZ()<<") at "
	   <<m->getPos().getTimestamp()<<" anchor:"<<m->getIsAnchor()<<"\n";
	delete msg;
	break;
    default:
      EV <<"Error! Got packet with unknown kind/type: " << m->kind()<<endl;
      delete msg;
    }
}

/**
 * A timer with kind = SEND_BROADCAST_TIMER indicates that a new
 * broadcast has to be send (@ref sendBroadcast). 
 *
 * There are no other timer implemented for this module.
 *
 * @sa sendBroadcast
 **/
void Centroid::handleSelfMsg(cMessage *msg) {}
void Centroid::handleTimer(unsigned int index) {
	LocPkt* pkt;
	double x=0, y=0, z=0, xReal, yReal, zReal;
	Coord pos;
	Location *loc;

	EV <<"handelTimer - Node: "<<id<<" Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
	EV<<"Msg kind: "<<index<<" SEND_ANCHOR_POS_TIMER: "<<SEND_ANCHOR_POS_TIMER<<" SEND_NODE_LOC_TIMER: "<<SEND_NODE_LOC_TIMER<</*" NBANCHORS: "<<NBANCHORS<<*/"\n";

	switch(index){
  	  case SEND_ANCHOR_POS_TIMER: // Anchor timer: send location
	    if(/*id>=NBANCHORS*/ !isAnchor){EV << "Non anchor node got SEND_ANCHOR_POS_TIMER message (this should never happen)\n"; exit(-1);}
	    if(baseUtility){pos=baseUtility->getPos();}
	    else{EV << "No submodule \"baseUtility\" found\n"; exit(-1);}
	    EV << "Anchor "<<id<<" timer rang: anchor sends its position "<<id<<"("<<pos.getX()<<","<<pos.getY()<<","<<pos.getZ()<<")\n";
	    // do a broadcast
	    pkt = new LocPkt("ANCHOR_BROADCAST_MESSAGE", ANCHOR_BROADCAST_MESSAGE);
	    loc = new Location(pos,simTime(),1.0);
	    pkt->setPos(*loc);
	    pkt->setId(id);
	    pkt->setIsAnchor(true);
	    sendBroadcast(pkt);
	    // Re-init anchor timer
	    setTimer(SEND_ANCHOR_POS_TIMER,ANCHOR_TIMER_INTERVAL);
	    break;
	  case SEND_NODE_LOC_TIMER: // Node timer: calculate location
	    if(isAnchor){EV << "Anchor node got SEND_NODE_LOC_TIMER message (this should never happen)\n"; exit(-1);}
	    EV << "Node "<<id<<" timer rang: should calculate position\n";
	    // Enough anchor positions? Then estimation location, otherwise wait for more
	    nb_anchor_positions=anchors.size();
	    if(nb_anchor_positions>=MIN_ANCHOR_POSITIONS){
	      EV << "Node "<<id<<" has heard enough anchors: "<<nb_anchor_positions<<"\n";
	      list<NodeInfo *>::const_iterator current;
	      EV << "Anchor neighbors(" << nb_anchor_positions <<") of node " << id << ": " << endl;
	      // calculate position
	      for (current = anchors.begin(); current != anchors.end(); current++) {
		EV_clear << "\t" << (*current)->id << " <" << (*current)->pos.getX() << "," << (*current)->pos.getY() << ">:" << (*current)->pos.getTimestamp() << endl;
		x += (*current)->pos.getX();
		y += (*current)->pos.getY();
		z += (*current)->pos.getZ();
	      }
	      x /= nb_anchor_positions;
	      y /= nb_anchor_positions;
	      z /= nb_anchor_positions;
	      // Get real position (ground truth)
	      if(baseUtility){pos=baseUtility->getPos(); xReal=pos.getX();yReal=pos.getY();zReal=pos.getZ();}
	      else{EV << "No submodule \"baseUtility\" found\n"; exit(-1);}
	      EV << "Node "<<id<<" estimated position is: ("<<x<<","<<y<<","<<z<<") with "<<nb_anchor_positions<<" anchors heard. True position is:  ("<<xReal<<","<<yReal<<","<<zReal<<")\n";
	      // do a broadcast
	      pos = new Coord(x,y,z);
	      pkt = new LocPkt("NODE_BROADCAST_MESSAGE", NODE_BROADCAST_MESSAGE);
	      loc = new Location(pos,simTime(),0.0);
	      pkt->setPos(*loc);
	      pkt->setId(id);
	      pkt->setIsAnchor(false);
	      sendBroadcast(pkt);
	    }
	    else{
	      EV << "Node "<<id<<" has heard too few anchors: "<<nb_anchor_positions<<"\n";
	    }
	    // Re-init node timer
	    setTimer(SEND_NODE_LOC_TIMER,NODE_TIMER_INTERVAL);
	    break;
	  default:
	    EV << "Unkown timer rang -> delete, index: "<<index<<endl;
	}
	
}

/**
 * This function creates a new broadcast message and sends it down to
 * the network layer
 **/
void Centroid::sendBroadcast(LocPkt *pkt)
{
    pkt->setLength(headerLength);
    // set the control info to tell the network layer the layer 3 address;
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    EV << "Sending broadcast packet!\n";
    sendDown( pkt );
}

void Centroid::finish() 
{
    BaseLocalization::finish();
    EV << "\t\tEnding centroid localization...\n";	
}
