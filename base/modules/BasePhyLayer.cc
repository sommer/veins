/***************************************************************************
 * file:        BasePhyLayer.cc
 *
 * author:      Marc Loebbers
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
 ***************************************************************************/


#include "BasePhyLayer.h"
#include <assert.h>
#include "FindModule.h"
#include "NicControlType.h"

Define_Module(BasePhyLayer);

void BasePhyLayer::initialize(int stage)
{
	BaseLayer::initialize(stage);
	if (stage ==1){ // 0 has to be there for others (i.e. BasePropagation) to do coreDebug
    	Timer::init(this);
        pm = FindModule<BasePropagation*>::findGlobalModule();
        if( pm == NULL )
			error("Could not find propagationmodel module");
     	hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;

        headerLength = par("headerLength");
        
		pm->registerNic(this);
		allocateTimers(1);
    }
}

void BasePhyLayer::handleLowerMsg(cMessage *msg)
{
    sendUp( decapsMsg(dynamic_cast<AirFrame *>(msg)) );
}

void BasePhyLayer::handleTimer(unsigned int index)
{
	coreEV << "transmission over" << endl;
    sendControlUp(new cMessage("TRANSMISSION_OVER", NicControlType::TRANSMISSION_OVER));
}

/**
 * This function encapsulates messages from the upper layer into an
 * AirFrame, copies the type and channel fields, adds the
 * headerLength, sets the pSend (transmitterPower) and returns the
 * AirFrame.
 */
AirFrame *BasePhyLayer::encapsMsg(cMessage *msg)
{
	AirFrame *frame = new AirFrame(msg->name(), msg->kind());
	frame->setLength(headerLength);
	frame->encapsulate(msg);
	return frame;
}

/**
 *
 **/
cMessage* BasePhyLayer::decapsMsg(AirFrame* frame)
{
	cMessage *m = frame->decapsulate();

	// delete the AirFrane
	delete frame;

	// retrun the MacPkt
	return m;
}

/**
 * Convenience function which calls sendToChannel with delay set
 * to 0.0.
 *
 * It also schedules the Timer which indicates the end of
 * transmission to upper layers.
 *
 * @sa sendToChannel
 */
void BasePhyLayer::sendDown(AirFrame *msg)
{
    coreEV << "sending msg to channel with encapsulated "<<msg->encapsulatedMsg()<<endl;
	assert(msg->encapsulatedMsg()!=NULL);
	pm->sendToChannel(this,msg);
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to the channel.
 *
 * The MAC frame is already encapsulated in an AirFrame and all standard
 * header fields are set.
 */
void BasePhyLayer::handleUpperMsg(cMessage *msg)
{
    AirFrame *frame = encapsMsg(msg);

    setTimer(0,frame->getDuration());
    sendDown(frame);
}
