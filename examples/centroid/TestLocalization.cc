/* -*- mode:c++ -*- ********************************************************
 * file:        TestLocalization.cc
 *
 * author:      Peterpaul Klein Haneveld
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
 * description: test class for the localization module
 **************************************************************************/

/* ************************************************************************
 * Aline Baggio:
 **************************************************************************
 * This file is an adapted copy of locNetwork/TestLocAppl and 
 * implements a Centroid localization algorithm (with moving anchors, 
 * see omnetpp.ini file)
 **************************************************************************/

#include "TestLocalization.h"
#include "NetwControlInfo.h"
#include <SimpleAddress.h>
//#include <BaseMobility.h>
#include <ConstSpeedMobility.h>

Define_Module_Like(TestLocalization, BaseLocalization);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we will set a timer to indicate the first time we will send a
 * message
 *
 **/
void TestLocalization::initialize(int stage)
{
  int id;
    BaseLocalization::initialize(stage);
    id = findHost()->par("id");
    if(stage == 0) {
      EV <<"initialize - Stage 0, id: "<<id<<"\n";
      EV <<"initialize - Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
      if(isAnchor) { /* Node is an anchor */
	EV << "\t\tInitializing centroid application for anchor "<<id<<"...\n";
	anchorTimer = new LocPkt( "anchor-timer", LOCALIZATION_MSG );
	anchorTimer->setType(SEND_ANCHOR_POS_TIMER);
      }
      else{ /* Node is a regular node */
	EV << "\t\tInitializing centroid application for node "<<id<<"...\n";
	nodeTimer = new LocPkt( "node-timer", LOCALIZATION_MSG );
	nodeTimer->setType(SEND_NODE_LOC_TIMER);
      }
      nb_anchor_positions=0;
      for(int i=0;i<NBANCHORS;i++){anchor_positions[i].id=-1;}
    }
    
    else if(stage==1) 
      {
      EV <<"initialize - Stage 1, id: "<<id<<"\n";
      if(isAnchor){scheduleAt(simTime() + findHost()->index() + ANCHOR_TIMER_INTERVAL, anchorTimer);} /* Init timer for anchors only */
      else{scheduleAt(simTime() + findHost()->index() + NODE_TIMER_INTERVAL, nodeTimer);} /* Init timer for nodes */
    }

}

/**
 * There are two kinds of messages that can arrive at this module: The
 * first (kind = ANCHOR_BROADCAST_MESSAGE) is a broadcast packet from a
 * neighbor node to which we have to send a reply. The second (kind =
 * NODE_POSITION_MESSAGE) is a reply to a broadcast packet that we
 * have send and just causes some output before it is deleted
 **/
void TestLocalization::handleLowerMsg( cMessage* msg )
{
    PositionPkt *m;
    LocPkt *pkt;

    int id;
    simtime_t ts = simTime();
    int anchorId;
    double anchorX, anchorY, anchorZ;

    id = findHost()->par("id");
    EV <<"handleLowerMsg - Node: "<<id<<" Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
    pkt = static_cast<LocPkt *>(msg);  

    switch(pkt->getType()){
    case ANCHOR_BROADCAST_MESSAGE:
        m = static_cast<PositionPkt *>(msg);        
	EV << "Received a broadcast packet from anchor "<<m->getId()<<" ["<<m->getSrcAddr()<<"]\n";
	if(isAnchor){/* anchor only stuff */
	  EV << "Anchor "<<id<<" got anchor position message with contents: "<<m->getId()<<"("<<m->getXPos()<<","<<m->getYPos()<<","<<m->getZPos()<<") at "<<ts<<"\n";
	}
	else {/* node only stuff */
	  anchorId=m->getId();
	  anchorX=m->getXPos();
	  anchorY=m->getYPos();
	  anchorZ=m->getZPos();
	  EV << "Node "<<id<<" heard anchor: "<<anchorId<<"("<<anchorX<<","<<anchorY<<","<<anchorZ<<") at "<<ts<<"\n";
	  if(anchor_positions[anchorId].id==-1){ // if never got message from this anchor yet
	    nb_anchor_positions++;
	    EV << "Node "<<id<<" heard a new anchor: "<<anchorId<<"(nb anchors heard: "<<nb_anchor_positions<<")\n";
	    anchor_positions[anchorId].id= anchorId;
	    anchor_positions[anchorId].x = anchorX;
	    anchor_positions[anchorId].y = anchorY;
	    anchor_positions[anchorId].z = anchorZ;
	    anchor_positions[anchorId].ts= ts;
	  }
	  else{
	    EV << "Node "<<id<<" has heard a known anchor: "<<anchorId<<"(nb anchors heard: "<<nb_anchor_positions<<")\n";
	    // Store anchor position (if it's not a deprecated message (i.e. we already got a newer position for this anchor))
	    if(ts>=anchor_positions[anchorId].ts){
	      anchor_positions[anchorId].x = anchorX;
	      anchor_positions[anchorId].y = anchorY;
	      anchor_positions[anchorId].z = anchorZ;
	      anchor_positions[anchorId].ts= ts;
	    }
	  }
	}
	delete msg;
        break;
    case NODE_POSITION_MESSAGE:
        m = static_cast<PositionPkt *>(msg);
	EV << "Node/anchor "<<id<<" got node position message with contents: "<<m->getId()<<"("<<m->getXPos()<<","<<m->getYPos()<<","<<m->getZPos()<<") at "<<ts<<"\n";
	delete msg;
	break;
    default:
      EV <<"Error! Got packet with unknown kind/type: " << pkt->kind()<<"/"<<pkt->getType()<<endl;
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
void TestLocalization::handleSelfMsg(cMessage *msg) {
	PositionPkt* pkt;
	LocPkt *p;
	double x=0, y=0, z=0, xReal, yReal, zReal;
	/*cModule*/ BaseUtility *utility=NULL;
	int id;
	Coord pos;

	id = findHost()->par("id");
	EV <<"handleSelfMsg - Node: "<<id<<" Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";

	EV<<"Msg kind: "<<msg->kind()<<"\n";
	EV<<"SEND_ANCHOR_POS_TIMER: "<<SEND_ANCHOR_POS_TIMER<<" SEND_NODE_LOC_TIMER: "<<SEND_NODE_LOC_TIMER<<" NBANCHORS: "<<NBANCHORS<<"\n";

	p = static_cast<LocPkt *>(msg);  

	switch(p->getType()){
  	  case SEND_ANCHOR_POS_TIMER: // Anchor timer: send location
	    if(/*id>=NBANCHORS*/ !isAnchor){EV << "Non anchor node got SEND_ANCHOR_POS_TIMER message (this should never happen)\n"; exit(-1);}
	    if((utility=(BaseUtility *)(findHost()->submodule("utility"))))//{x=utility->par("x");y=utility->par("y");y=utility->par("z");}
	      //if(utility=(BaseUtility *)(getNodeModule("utility")))
	       //GET REAL LOC HERE! Don't read form omnet.ini!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{pos=utility->getPos(); x=pos.getX();y=pos.getY();z=pos.getZ();}
	    else{EV << "No submodule \"utility\" found\n"; exit(-1);}
	    EV << "Timer rang: anchor sends its position"<<id<<"("<<x<<","<<y<<","<<z<<")\n";
	    // do a broadcast
	    pkt = new PositionPkt("ANCHOR_BROADCAST_MESSAGE", LOCALIZATION_MSG);
	    pkt->setXPos(x);
	    pkt->setYPos(y);
	    pkt->setZPos(z);
	    pkt->setId(id);
	    pkt->setType(ANCHOR_BROADCAST_MESSAGE);
	    sendBroadcast(pkt);
	    // Re-init anchor timer
	    scheduleAt(simTime() + findHost()->index() + ANCHOR_TIMER_INTERVAL, anchorTimer);
	    break;
	  case SEND_NODE_LOC_TIMER: // Node timer: calculate location
	    if(isAnchor){EV << "Anchor node got SEND_NODE_LOC_TIMER message (this should never happen)\n"; exit(-1);}
	    EV << "Node "<<id<<"timer rang: should calculate position\n";
	    // Enough anchor positions? Then estimation location, otherwise wait for more
	    if(nb_anchor_positions>=MIN_ANCHOR_POSITIONS){
	      EV << "Node "<<id<<" has heard enough anchors: "<<nb_anchor_positions<<"\n";
	      // calculate position
	      for(int i=0;i<NBANCHORS;i++){
		if(!(anchor_positions[i].id==-1)){
		  EV << "Node "<<id<<" has heard anchor "<<anchor_positions[i].id<<"("<<anchor_positions[i].x<<","<<anchor_positions[i].y<<","<<anchor_positions[i].z<<")\n";
		  x += anchor_positions[i].x;
		  y += anchor_positions[i].y;
		  z += anchor_positions[i].z;
		}
	      }
	      x /= nb_anchor_positions;
	      y /= nb_anchor_positions;
	      z /= nb_anchor_positions;
	      // Get real position (ground truth)
	      if((utility=(BaseUtility *)findHost()->submodule("utility")))
		//{xReal=utility->par("x");yReal=utility->par("y");zReal=utility->par("z");}
		{pos=utility->getPos(); xReal=pos.getX();yReal=pos.getY();zReal=pos.getZ();}
	      else{EV << "No submodule \"utility\" found\n"; exit(-1);}
	      EV << "Node "<<id<<" estimated position is: ("<<x<<","<<y<<","<<z<<") with "<<nb_anchor_positions<<" anchors heard. True position is:  ("
		 <<xReal<<","<<yReal<<","<<zReal<<")\n";
	      // reset (will wait for new anchor positions)
	      nb_anchor_positions=0;
	      for(int i=0;i<NBANCHORS;i++){anchor_positions[i].id=-1;}
	    }
	    else{
	      EV << "Node "<<id<<" has heard too few anchors: "<<nb_anchor_positions<<"\n";
	    }
	    // do a broadcast
	    pkt = new PositionPkt("NODE_POSITION_MESSAGE", LOCALIZATION_MSG);
	    pkt->setXPos(x);
	    pkt->setYPos(y);
	    pkt->setZPos(z);
	    pkt->setId(id);
	    pkt->setType(NODE_POSITION_MESSAGE);
	    sendBroadcast(pkt);
	    // Re-init node timer
	    scheduleAt(simTime() + findHost()->index() + NODE_TIMER_INTERVAL, nodeTimer);
	    break;
	  default:
	    EV << "Unkown timer rang -> delete, kind: "<< p->kind()<<"/"<<p->getType()<<endl;
	    delete msg;
	}
	
}

/**
 * This function creates a new broadcast message and sends it down to
 * the network layer
 **/
void TestLocalization::sendBroadcast(PositionPkt *pkt)
{
    pkt->setDestAddr(-1);
    // we use the host modules index() as a appl address
    pkt->setSrcAddr(findHost()->index()); // We use the node module index as address
    pkt->setLength(headerLength);
    
    // set the control info to tell the network layer the layer 3
    // address;
    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
    EV << "Sending broadcast packet!\n";
    sendDown( pkt );
}

void TestLocalization::finish() 
{
    BaseLocalization::finish();
    if(anchorTimer){if(!anchorTimer->isScheduled()) delete anchorTimer;}
    if(nodeTimer){if(!nodeTimer->isScheduled()) delete nodeTimer;}

    EV << "\t\tEnding centroid localization...\n";	
}

//Coord TestLocalization::getLoc()
//{
//  EV << "Providing position estimation: id:"<<findHost()->par("id")<<"(x:"<<findHost()->par("xEst")<<",y:"<<findHost()->par("yEst")
//     <<",z:"<<findHost()->par("zEst")<<") ts:"<<findHost()->par("ts")<<"\n";	
//}
