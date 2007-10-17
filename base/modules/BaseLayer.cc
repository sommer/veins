/***************************************************************************
 * file:        BaseLayer.cc
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
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
 * description: basic MAC layer class
 *              subclass to create your own MAC layer
 ***************************************************************************
 * changelog:   $Revision: 250 $
 *              last modified:   $Date: 2006-04-04 18:53:02 +0200 (Tue, 04 Apr 2006) $
 *              by:              $Author: koepke $
 **************************************************************************/


#include "BaseLayer.h"
#include <assert.h>

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BaseModule.
 * This module takes care of the gate initialization.
 *
 **/
void BaseLayer::initialize(int stage)
{
    BaseModule::initialize(stage);
    if(stage==0){
		if (hasPar("stats") && par("stats").boolValue())
		{
			incoming = new std::map<MsgType,std::map<int,std::pair<char *,int>*> *>();
			outgoing = new std::map<MsgType,std::map<int,std::pair<char *,int>*> *>();
			doStats = true;
		}
		else
			doStats = false;

        uppergateIn  = findGate("uppergateIn");
        uppergateOut = findGate("uppergateOut");
        lowergateIn  = findGate("lowergateIn");
        lowergateOut = findGate("lowergateOut");
        upperControlIn  = findGate("upperControlIn");
        upperControlOut = findGate("upperControlOut");
        lowerControlIn  = findGate("lowerControlIn");
        lowerControlOut = findGate("lowerControlOut");

    }
}


/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleUpperMsg, handleLowerMsg, handleSelfMsg
 **/
void BaseLayer::handleMessage(cMessage* msg)
{
    if(msg->arrivalGateId()==uppergateIn) {
		recordPacket(true,UPPER_DATA,msg);
        handleUpperMsg(msg);
    }
    else if(msg->arrivalGateId()==upperControlIn) {
		recordPacket(true,UPPER_CONTROL,msg);
        handleUpperControl(msg);
    }
    else if(msg->arrivalGateId()==lowerControlIn){
		recordPacket(true,LOWER_CONTROL,msg);
        handleLowerControl(msg);
    }
    else if (msg->isSelfMessage()){
        handleSelfMsg(msg);
    }
    else { // default assumption is data in from somewhere else, which if it's got a funny name
		   // is probably from a random propagation layer
		recordPacket(true,LOWER_DATA,msg);
        handleLowerMsg(msg);
    }
}

void BaseLayer::sendDown(cMessage *msg) {
	recordPacket(false,LOWER_DATA,msg);
    send(msg, lowergateOut);
}

void BaseLayer::sendUp(cMessage *msg) {
	recordPacket(false,UPPER_DATA,msg);
    send(msg, uppergateOut);
}

void BaseLayer::sendControlUp(cMessage *msg) {
	recordPacket(false,UPPER_CONTROL,msg);
    send(msg, upperControlOut);
}

void BaseLayer::sendControlDown(cMessage *msg) {
	recordPacket(false,LOWER_CONTROL,msg);
    send(msg, lowerControlOut);
}

void BaseLayer::recordPacket(bool in, MsgType type, const cMessage * msg)
{
	if (!doStats)
		return;
	std::map<MsgType,std::map<int,std::pair<char *,int>* > *> *use;
	if (in)
		use = incoming;
	else
		use = outgoing;
	if (use->find(type)==use->end())
		(*use)[type] = new std::map<int,std::pair<char *,int> *>();

	std::map<int,std::pair<char *, int> *> *count = (*use)[type];

	if (count->find(msg->kind())==count->end())
		(*count)[msg->kind()] = new std::pair<char *,int>(NULL,0);
	(*count)[msg->kind()]->second++;
	(*count)[msg->kind()]->first = strdup(msg->name());
}

void typeToChar(std::ostream& str, MsgType type,bool in)
{
	const char * dir = in?" from ":" to ";
	switch(type)
	{
		case UPPER_DATA:
			str << "Data" << dir << "upper layer";
			return;
		case LOWER_DATA:
			str << "Data" << dir << "lower layer";
			return;
		case UPPER_CONTROL:
			str << "Control packets" << dir << "upper layer";
			return;
		case LOWER_CONTROL:
			str << "Control packets" << dir << "lower layer";
			return;
	}
	throw new cException();
}

#define statsEV ev << "STATS : " <<logName() << "::" << className() << ": "

void BaseLayer::printPackets(std::map<MsgType,std::map<int,std::pair<char *,int>* > *> *use, bool in)
{
	assert(doStats);
	for (std::map <MsgType, std::map<int,std::pair<char *, int> *>* >::iterator pi = use->begin(); pi != use->end(); pi++)
	{
		statsEV << "\ttype ";
		typeToChar(ev,pi->first,in);
		ev<<endl;
		for (std::map <int,std::pair<char *, int> *>::iterator ki = pi->second->begin(); ki != pi->second->end(); ki++)
		{
			statsEV << "\t\t"<<ki->second->second << " x ";
			if (ki->second->first == NULL)
				ev << ki->first<<endl;
			else	
				ev << ki->second->first<< endl;
		}
	}
}

void BaseLayer::finish()
{
	if (!doStats)
		return;
	statsEV << "Incoming packets" <<endl;
	printPackets(incoming,true);
	statsEV << "Outgoing packets"<<endl;
	printPackets(outgoing, false);
}

BaseLayer::~BaseLayer()
{
	if (!doStats)
		return;
	delete incoming;
	delete outgoing;
}
