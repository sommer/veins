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
        
		pm->registerNic((BaseModule*)parentModule());
		allocateTimers(1);
    }
}

void BasePhyLayer::handleLowerMsg(cMessage *msg)
{
	// msg must come from channel
	handleLowerMsgStart(msg);
	bufferMsg(msg);
}

void BasePhyLayer::handleTimer(unsigned int index)
{
	if (index == 0)
	{
		coreEV << "transmission over" << endl;
        sendControlUp(new cMessage("TRANSMISSION_OVER", NicControlType::TRANSMISSION_OVER));
	}
	else
	{
		coreEV << "frame is completely received now\n";
		// unbuffer the message
	    cMessage *frame = static_cast<cMessage *>(contextPointer(index));
		handleLowerMsgEnd(frame);
		deleteTimer(index);
	}
}


/**
 * The packet is put in a buffer for the time the transmission would
 * last in reality. A timer indicates when the transmission is
 * complete. So, look at unbufferMsg to see what happens when the
 * transmission is complete..
 */
void BasePhyLayer::bufferMsg(cMessage *msg)
{
	AirFrame *frame = static_cast<AirFrame *>(msg);

	// set timer to indicate transmission is complete
	unsigned int timer = setTimer(frame->getDuration());
	assert(msg->encapsulatedMsg()!=NULL);
	setContextPointer(timer,msg);
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
void BasePhyLayer::sendDown(cMessage *msg)
{
    coreEV << "sending msg to channel with encapsulated "<<msg->encapsulatedMsg()<<endl;
	assert(msg->encapsulatedMsg()!=NULL);
	pm->sendToChannel((BaseModule*)parentModule(),msg, 0.0);
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
    sendDown(static_cast<cMessage *>(frame));
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This function is called right after a message is received,
 * i.e. right before it is buffered for 'transmission time'.
 *
 * Here you should decide whether the message is "really" received or
 * whether it's receive power is so low that it is just treated as
 * noise.
 *
 **/
void BasePhyLayer::handleLowerMsgStart(cMessage *msg)
{
    coreEV <<"in handleLowerMsgStart"<<endl;
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * Do not forget to send the message to the upper layer with sendUp()
 */
void BasePhyLayer::handleLowerMsgEnd(cMessage *msg)
{
    coreEV << "in handleLowerMsgEnd\n";
    sendUp( decapsMsg(static_cast<AirFrame *>(msg)) );
}
