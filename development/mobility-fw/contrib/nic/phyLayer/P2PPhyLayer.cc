/***************************************************************************
 * file:        P2PPhyLayer.cc
 *
 * author:      Marc Loebbers, Daniel Willkomm
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
 * description: a very simple physical layer implementation
 ***************************************************************************/


#include "P2PPhyLayer.h"

#include <ChannelControl.h>

#include "MacPkt_m.h"

Define_Module( P2PPhyLayer);


/**
 * First we have to initialize the module from which we derived ours,
 * in this case ChannelAccess.
 *
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 **/
void P2PPhyLayer::initialize(int stage)
{
    ChannelAccess::initialize(stage);

    if(stage==0){
        uppergateIn  = findGate("uppergateIn");
        uppergateOut = findGate("uppergateOut");
        upperControlOut = findGate("upperControlOut");
        
        headerLength=par("headerLength");
        bitrate=par("bitrate");
    
        transmitterPower = par("transmitterPower");

        if(hasPar("pBit"))
            pBit = par("pBit");
        else{
            EV << "Warning: no pBit specified, set pBit=-1!\n";
            pBit=-1;
        }
        // transmitter power CANNOT be greater than in ChannelControl
        if (transmitterPower > (double) (cc->par("pMax")))
            error("tranmitterPower cannot be bigger than pMax in ChannelControl!");
        
        catActiveChannel = bb->subscribe(this, &channel, parentModule()->id());
    }
    else {
        txOverTimer = new cMessage("txOverTimer");
    }
}

/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * Messages from the channel are also buffered here in order to
 * simulate a transmission delay
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleUpperMsg, handleLowerMsg, handleSelfMsg
 **/
void P2PPhyLayer::handleMessage(cMessage *msg)
{
    if(msg->arrivalGateId()==uppergateIn){
        //message from upper layer

	// we need the destId to find the destination gate we have to
	// send the msg to
        int destId = static_cast<MacPkt*>(msg)->getDestAddr();

        AirFrame* frame = encapsMsg( msg );
        handleUpperMsg( frame, destId );
    }
    else if(msg == txOverTimer) {
        EV << "transmission over" << endl;
        sendControlUp(new cMessage("TRANSMISSION_OVER", NicControlType::TRANSMISSION_OVER));
    }
    else if(msg->isSelfMessage()){
        if(msg->kind() == RECEPTION_COMPLETE) {
            EV << "frame is completely received now\n";
            // unbuffer the message
            AirFrame *frame = unbufferMsg(msg);
            handleLowerMsg(frame);
        }
        else
            //self message
            handleSelfMsg(msg);      
    }
    else{ 
        // msg must come from channel -> buffer till complete reception
        AirFrame *frame = static_cast<AirFrame *>(msg);
        bufferMsg(frame);
    }
}

/**
 * Encapsulate the received MacPkt into an AirFrame and set all
 * needed header fields.
 *
 * @sa AirFrame
 **/
AirFrame* P2PPhyLayer::encapsMsg(cMessage *msg)
{  
    AirFrame *frame = new AirFrame(msg->name(), msg->kind());
    frame->setPSend(transmitterPower);
    frame->setLength(headerLength);
    frame->setChannelId(channel.getActiveChannel());
    frame->encapsulate(msg);
    frame->setDuration(calcDuration(frame));
    frame->setHostMove(hostMove);
    return frame;
}

/**
 * Usually the duration is just the frame length divided by the
 * bitrate. However there may be cases (like 802.11) where the header
 * has a different modulation (and thus a different bitrate) than the
 * rest of the message.
 *
 * Just redefine this function in such a case!
 **/
double P2PPhyLayer::calcDuration(cMessage *af)
{
    double duration;
    duration = (double)af->length()/(double)bitrate;
    return duration;
}

/**
 * The packet is put in a buffer for the time the transmission would
 * last in reality. A timer indicates when the transmission is
 * complete. So, look at unbufferMsg to see what happens when the
 * transmission is complete..
 **/
void P2PPhyLayer::bufferMsg(AirFrame *frame)
{
    // set timer to indicate transmission is complete
    cMessage *timer = new cMessage(NULL, RECEPTION_COMPLETE);
    timer->setContextPointer(frame);
    scheduleAt(simTime()+(frame->getDuration()),timer);
}

/**
 * Get the context pointer to the now completely received AirFrame and
 * delete the self message
 **/
AirFrame* P2PPhyLayer::unbufferMsg( cMessage *msg)
{
    AirFrame *frame = static_cast<AirFrame *>(msg->contextPointer());
    //delete the self message
    delete msg;

    return frame;
}

/** 
 * Convenience function which calls sendDelayedP2P with delay set to
 * 0.0.
 *
 * @sa sendDelayedP2P
 **/
void P2PPhyLayer::sendP2P(AirFrame *msg, int dest)
{
    sendDelayedP2P( msg, 0.0, dest );
}

/** 
 * Convenience function which calls sendDelayedDown with delay set to
 * 0.0.
 *
 * @sa sendDelayedDown
 **/
void P2PPhyLayer::sendDown(AirFrame *msg, int dest)
{
    sendDelayedDown( msg, 0.0, dest );
}

/** 
 * Convenience function which calls sendDelayedUp with delay set to
 * 0.0.
 *
 * @sa sendDelayedUp
 **/
void P2PPhyLayer::sendUp(AirFrame *msg)
{
    sendDelayedUp( msg, 0.0 );
}

/**
 * Finds the destination gate of the recipient and sends the message
 * only via the link which is connected to that gate..
 *
 * This function is only used for the point-to-point sending and thus
 * the specialily of the P2P network.
 *
 * Although the wireless channel is a broadcast channel and all hosts
 * within interference distance should receive all messages sendP2P
 * sends a message directly via unicast to the desired destination.
 *
 * This is only possible because we do not assume lost messages due to
 * interference or collitions. All possible message losses are summed
 * up in one parameter @ref pBit.
 *
 * @sa ChannelControl::getOutGateTo()
 **/
void P2PPhyLayer::sendDelayedP2P(AirFrame* frame, double delay, int dest)
{
    const cGate *outGate = cc->getOutGateTo(parentModule()->id(), dest, &hostMove.startPos );
    if(!outGate){
	ev << "WARNING: " << logName() << " sendP2P: AirFrame lost because destination nic with id="
	  << dest <<" not connected"<<endl;
	delete frame;
	return;
    }
    else{

	if(useSendDirect){
	    //use sendDirect
	    sendDirect( (cMessage*)frame, delay, (cGate*) outGate );
	}
	else{
	    //use send
	    sendDelayed( (cMessage*)frame , delay, outGate->id());
	}
    }
}

/**
 * For this special case the sending of the message as broadcast and
 * unicast messages are handled differently.
 *
 * This function is only for convenience so that the programmer does
 * not have to take care about the details of message sending
 *
 * @param dest Destination nic module id() in order to find the
 * destination gate for sending the message
 **/
void P2PPhyLayer::sendDelayedDown(AirFrame* msg, double delay, int dest)
{
  if(dest == -1){
    //send broadcast msg
    EV <<"send broadcast message to all neighbors\n";
    sendToChannel( (cMessage*)msg, delay );
  }
  else{
    //send unicast msg
    EV <<"send unicast message to "<<dest<<endl;
    sendDelayedP2P( msg, delay, dest );
  }
}

/**
 * decapsulate message and send it to the upper layer. 
 *
 * to be called within @ref handleLowerMsg.
 **/
void P2PPhyLayer::sendDelayedUp(AirFrame* msg, double delay)
{
    cMessage *newPkt = msg->decapsulate();
    sendDelayed( newPkt, delay, uppergateOut);
    delete msg;
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This basic implementation just checks whether a received packet had
 * errors. Decision depends only on @ref pBit (defined in
 * omnetppp.ini) and the message length. The length field is assumed
 * to be in bits.
 *
 * If you want to forward the message to upper layers please use @ref
 * sendUp which will decapsulate the MacPkt before sending
 **/
void P2PPhyLayer::handleLowerMsg( AirFrame *msg )
{
    double pktError=pow(1-pBit,(int)msg->length());
    bool ok=bernoulli(pktError, 0);

    //send the packet up if it was received correctly - otherwise delete it
    if(ok){
        EV <<"msg "<<msg->name()<<" forwarded to upper layer\n";
        sendUp(msg);
    }
    else{
        EV <<"Sorry, msg "<<msg->name()<<" lost due to biterrors\n";
        delete msg;
    }
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to the channel.
 *
 * The MacPkt is already encapsulated in an AirFrame and all standard
 * header fields are set.
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of decapsulation and anything
 * else needed
 *
 * @param dest Destination nic module id() in order to find the
 * destination gate for sending the message
 **/
void P2PPhyLayer::handleUpperMsg(AirFrame* msg, int dest)
{
    scheduleAt(simTime() + msg->getDuration(), txOverTimer);
    sendDown(msg, dest);
}

/**
 * send a control message to the upper layer
 */
void P2PPhyLayer::sendControlUp(cMessage *msg)
{
    send(msg, upperControlOut);
}

void P2PPhyLayer::finish()
{
    if(!txOverTimer->isScheduled()) delete txOverTimer;
}

void P2PPhyLayer::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    ChannelAccess::receiveBBItem(category, details, scopeModuleId);
    if(category == catActiveChannel) {
        channel = *(static_cast<const ActiveChannel *>(details));
    }
}

