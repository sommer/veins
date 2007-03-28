/*
 *	copyright:   	(C) 2006 Computer Networks Group (CN) at
 *			University of Paderborn, Germany.
 *	
 *			This program is free software; you can redistribute it
 *			and/or modify it under the terms of the GNU General Public
 *			License as published by the Free Software Foundation; either
 *			version 2 of the License, or (at your option) any later
 *			version.
 *
 *			For further information see file COPYING
 *			in the top level directory.
 *
 *			Based on Mobility Framework 2.0p2 developed at 
 *			TKN (TU Berlin) and, ChSim 2.1 developed at CN 
 *			(Uni Paderborn).
 *
 *	file:		$RCSfile: StreamApplLayer.cc,v $
 *
 *      last modified:	$Date: 2007/02/22 13:41:54 $
 *      by:		$Author: tf $
 *
 *      informatin:	-
 *
 *	changelog:   	$Revision: 1.12 $
 *			$Log: StreamApplLayer.cc,v $
 *			Revision 1.12  2007/02/22 13:41:54  tf
 *			- changed delay selection to bigger interval (less collisions in init phase)
 *			
 *			Revision 1.11  2007/02/21 15:41:53  tf
 *			- reply is not sent as broadcast any longer
 *			
 *			Revision 1.10  2007/01/31 09:45:05  tf
 *			- added parameter streamRate to set rate of stream generation
 *			
 *			Revision 1.9  2007/01/29 22:47:00  tf
 *			- added a counter for more verbose output
 *			
 *			Revision 1.8  2007/01/29 18:09:48  tf
 *			*** empty log message ***
 *			
 *			Revision 1.7  2007/01/28 12:55:43  tf
 *			- only cosmetic changes
 *			
 *			Revision 1.6  2007/01/26 15:18:49  tf
 *			- still some timing issues left
 *			
 *			Revision 1.5  2007/01/24 16:35:10  tf
 *			- fixed rate for each terminal
 *			
 *			Revision 1.4  2007/01/21 20:36:15  tf
 *			- packet generation rate is altered by MAC buffer state, to prevent
 *			  empty MAC buffer
 *			- only ApplLayer with initBurst>0 acts as stream generator
 *			
 *			Revision 1.3  2007/01/19 15:43:54  tf
 *			- fixed timer initialization in StreamApplLayer
 *			- fixed MAC frame duration computation
 *			
 *			Revision 1.2  2007/01/19 14:50:44  tf
 *			- fixed stupid typo in function name
 *			
 *			Revision 1.1  2007/01/19 09:57:07  tf
 *			- added new Applayer for tests
 *				- params
 *					length of frames
 *				- sends a broadcast which is answered by all receiving stations
 *				  receiving stations reply and will be scheduled as stream dest.
 *			
 */


#include "StreamApplLayer.h"


Define_Module(StreamApplLayer);

// do some initialization 
void StreamApplLayer::initialize(int stage)
{
    TestApplLayer::initialize(stage);

    if(stage==0){
    	initBurst = par("initBurst");
	streamLength = par("streamLength");
	streamRate = par("streamRate");
	startTimer = new cMessage("START_STREAM",START_STREAM);

	bufferStateCat = bb->subscribe(dynamic_cast<ImNotifiable*>(this), new BBBufferState(), parentModule()->id());
	
	clientList.clear();

	headerLength=par("headerLength");

	randDelay = 5;
	EV<<"Stage 0 clear."<<endl;
    }
    else if (stage==1)
    {
	EV<<"Stage 1 clear."<<endl;
    }
}


void StreamApplLayer::handleSelfMsg(cMessage *msg)
{
    int someInt = 1;
    ClientList::iterator it;
    switch(msg->kind())
    {
    case SEND_BROADCAST_TIMER:
        if (initBurst>0)
		sendBroadcast();
        break;
    case START_STREAM:
    	EV<<"START_STREAM msg received."<<endl;
	for (int i=0;i<streamLength;i++) 
		for (it=clientList.begin();it!=clientList.end();it++) {
			EV<<"sending packet "<<someInt++<<endl;
			sendTo(*it);
		}
	while (startTimer->isScheduled())
		cancelEvent(startTimer);
	scheduleAt(simTime()+streamRate,startTimer);
	break;
    default:
        EV <<" Unkown selfmessage! -> delete, kind: "<<msg->kind()<<endl;
    }
}

void StreamApplLayer::handleLowerMsg(cMessage* msg)
{
    ApplPkt* pkt = static_cast<ApplPkt*>(msg);
    NetwControlInfo* nci = static_cast<NetwControlInfo*>(msg->removeControlInfo());

    switch(msg->kind()) 
    {
    case BROADCAST_MESSAGE:
    	EV<<"Received initial broadcast packet, replying to streaming server: "<<pkt->getSrcAddr()<<endl;
	sendReply(pkt,nci);
	break;

    case BROADCAST_REPLY_MESSAGE:
    	EV<<"Received broadcast reply, adding "<<pkt->getSrcAddr()<<", "<<nci->getNetwAddr()<<" to stream client list."<<endl;
	clientList.push_back(pkt->getSrcAddr());
	clientNetwAddresses[pkt->getSrcAddr()] = nci->getNetwAddr();
	if (initBurst)
	{
		if (startTimer->isScheduled())
			cancelEvent(startTimer);
		scheduleAt(simTime()+1,startTimer);
	}
	delete pkt;
	break;

    case STREAM_PACKET:
    	EV<<"Received stream packet."<<endl;
	delete pkt;
	break;

    default:
    	EV<<"Unknown packet type received "<<msg->kind()<<endl;
    }
}

void StreamApplLayer::sendTo(int addr)
{
    ApplPkt* pkt = new ApplPkt("STREAM_PACKET",STREAM_PACKET);
    NetwControlInfo* nci = new NetwControlInfo(clientNetwAddresses[addr]);
    pkt->setDestAddr(addr);
    // we use the host modules index() as a appl address
    pkt->setSrcAddr(myApplAddr());
    pkt->setLength(headerLength);
    //pkt->setLength(headerLength+static_cast<int>(uniform(-256,256))); 
    pkt->setControlInfo(nci);
    EV<<"Sending stream packet to "<<addr<<endl;
    sendDown(pkt);
}

void StreamApplLayer::receiveBBItem(int category, BBItem *details, int scopeModuleId)
{
    BBBufferState* state;
    state = static_cast<BBBufferState*>(details);
    bool scheduleNext = true;

    ev<<"Received BBBufferState ";
    if (state->getBufferState() == EMPTY) {
    	randDelay = 0.01;
	ev<<"EMPTY";
    } else if (state->getBufferState() == LOW) {
    	randDelay = 0.1;
	scheduleNext = true;
	ev<<"LOW";
    } else if (state->getBufferState() == HIGH) {
    	randDelay = 1;
	ev<<"HIGH";
    } else if (state->getBufferState() == FULL) {
	while (startTimer->isScheduled())
		cancelEvent(startTimer);
	scheduleNext=false;
	ev<<"FULL";
    }
    ev<<" randDelay set to "<<randDelay<<endl;
/*    if (initBurst) {
    if (startTimer->isScheduled())
    	cancelEvent(startTimer);
    if (scheduleNext)
        scheduleAt(simTime()+0.001,startTimer);
    } */
}

void StreamApplLayer::finish() 
{
     TestApplLayer::finish();
     if (startTimer->isScheduled())
     	cancelEvent(startTimer);
     delete startTimer;
}

void StreamApplLayer::sendReply(ApplPkt* msg, NetwControlInfo* nci) {
    double delay;

    delay = uniform(0, 0.1);
    
    msg->setControlInfo(nci);
    msg->setDestAddr(msg->getSrcAddr());
    msg->setSrcAddr(myApplAddr());
    msg->setKind(BROADCAST_REPLY_MESSAGE);
    msg->setName("BROADCAST_REPLY_MESSAGE");
    sendDelayedDown(msg, delay);

    EV << "sent message with delay " << delay << endl;
} 

