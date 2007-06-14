/***************************************************************************
 * file:        TestLocAppl.cc
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
 * description: application layer: test class for the application layer
 ***************************************************************************/

/* ************************************************************************
 * Peterpaul Klein Haneveld:
 **************************************************************************
 * This file is essentially a copy of TestApplLayer with just the following
 * replacements:
 *
 * TestApplLayer        -> TestLocAppl
 * BaseApplLayer        -> BaseLocAppl
 **************************************************************************/

/* ************************************************************************
 * Aline Baggio:
 **************************************************************************
 * Simplied application doing clodse to nothing. 
 * Should only ask for position estimation on a regular basis (missing bit)
 **************************************************************************/

#include "TestLocAppl.h"
#include "NetwControlInfo.h"

#include <SimpleAddress.h>

Define_Module_Like(TestLocAppl, BaseLocAppl);


/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we will set a timer to indicate the first time we will send a
 * message
 *
 **/
void TestLocAppl::initialize(int stage)
{
  int id;
    BaseLocAppl::initialize(stage);
    id = grandparentModule()->par("id");

    EV <<"Initializing application\n";
    if(stage == 0) {
      EV <<"initialize - apllication - Stage 0, id: "<<id<<"\n";
      EV <<"initialize - apllication - Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";
      appTimer = new cMessage( "app-timer", GET_LOC_INFO );
    }
    else if(stage==1) 
      {
      EV <<"initialize - apllication - Stage 1, id: "<<id<<"\n";
      scheduleAt(simTime() + findHost()->index() + GET_LOC_INFO_TIMER_INTERVAL, appTimer);
    }

}


/**
 * There are two kinds of messages that can arrive at this module: The
 * first (kind = ANCHOR_BROADCAST_MESSAGE) is a broadcast packet from a
 * neighbor node to which we have to send a reply. The second (kind =
 * NODE_POSITION_MESSAGE) is a reply to a broadcast packet that we
 * have send and just causes some output before it is deleted
 **/
void TestLocAppl::handleLowerMsg( cMessage* msg )
{
    EV <<"handleLowerMsg - application\n";
}

/**
 * A timer with kind = SEND_BROADCAST_TIMER indicates that a new
 * broadcast has to be send (@ref sendBroadcast). 
 *
 * There are no other timer implemented for this module.
 *
 * @sa sendBroadcast
 **/
void TestLocAppl::handleSelfMsg(cMessage *msg) {
  int id;
  id = grandparentModule()->par("id");
  EV <<"handleSelfMsg - application - Node: "<<id<<" Name: "<<fullName()<< " Path: "<<fullPath()<<"\n";

    switch( msg->kind() ){

    case GET_LOC_INFO:
      EV <<"handleSelfMsg - application - Node: "<<id<<"got a GET_LOC_INFO\n";
      //getLoc(); // should get position estimation HERE!
      scheduleAt(simTime() + findHost()->index() + GET_LOC_INFO_TIMER_INTERVAL, appTimer);
	break;
    default:
    	EV << "Unkown selfmessage! -> delete, kind: "<<msg->kind() <<endl;
	delete msg;
    }
}

///**
// * This function creates a new broadcast message and sends it down to
// * the network layer
// **/
void TestLocAppl::sendBroadcast()
{
//    pkt->setDestAddr(-1);
//    // we use the host modules index() as a appl address
//    pkt->setSrcAddr(myApplAddr());
//    pkt->setLength(headerLength);
//    
//    // set the control info to tell the network layer the layer 3
//    // address;
//    pkt->setControlInfo( new NetwControlInfo(L3BROADCAST) );
//    EV << "Sending broadcast packet!\n";
//    sendDown( pkt );
}


//void TestLocAppl::sendReply(ApplPkt *msg) 
//{
//    double delay;
//
//    delay = uniform(0, 0.01);
//
//    msg->setDestAddr(msg->getSrcAddr());
//    msg->setSrcAddr(myApplAddr());
//    msg->setKind(NODE_POSITION_MESSAGE);
//    msg->setName("NODE_POSITION_MESSAGE");
//    sendDelayedDown(msg, delay);
//
//    EV << "sent message with delay " << delay << endl;
//
//    //NOTE: the NetwControl info was already ste by the network layer
//    //and stays the same
//}

void TestLocAppl::finish() 
{
    BaseLocAppl::finish();
    EV << "\t\tEnding application...\n";	
}
