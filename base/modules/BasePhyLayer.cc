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

//const double BasePhyLayer::speedOfLight = ChannelControl::speedOfLight;

Define_Module(BasePhyLayer);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case ChannelAccess.
 *
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 *
 * If you want to use your own AirFrames you have to redefine createCapsulePkt
 * function.
 */
void BasePhyLayer::initialize(int stage)
{
    ChannelAccess::initialize(stage);

    if (stage == 1){
        uppergateIn = findGate("uppergateIn");
        uppergateOut = findGate("uppergateOut");
        upperControlOut = findGate("upperControlOut");
        upperControlIn = findGate("upperControlIn");

        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;

        headerLength = par("headerLength");

        hasPar("transmitterPower") ? transmitterPower=par("transmitterPower") :
            transmitterPower = static_cast<double>(cc->par("pMax"));
        hasPar("sensitivity") ? sensitivity=FWMath::dBm2mW(par("sensitivity")) :
            sensitivity = FWMath::dBm2mW(static_cast<double>(cc->par("sat")));
        hasPar("carrierFrequency") ? carrierFrequency=par("carrierFrequency") :
            carrierFrequency = static_cast<double>(cc->par("carrierFrequency"));
        hasPar("alpha") ? alpha=par("alpha") :
            alpha = static_cast<double>(cc->par("alpha"));

		bitrate = par("bitrate");
        
        //catActiveChannel = bb->subscribe(this, &channel, parentModule()->id());
    }
    else if (stage == 2){
        if(alpha < static_cast<double>(cc->par("alpha")))
            error("SnrEval::initialize() alpha can't be smaller than in \
                   ConnectionManager. Please adjust your omnetpp.ini file accordingly");
        if(transmitterPower > static_cast<double>(cc->par("pMax")))
            error("SnrEval::initialize() transmitterPower (%f) can't be bigger than \
                   pMax (%f) in ConnectionManager! Please adjust your omnetpp.ini file accordingly",transmitterPower);
        if(sensitivity < FWMath::dBm2mW(static_cast<double>(cc->par("sat"))))
            error("SnrEval::initialize() sensitivity can't be smaller than the signal attentuation threshold (sat) in \
                   ConnectionManager. Please adjust your omnetpp.ini file accordingly");
        if(carrierFrequency < static_cast<double>(cc->par("carrierFrequency")))
            error("SnrEval::initialize() carrierFrequency can't be smaller than in \
                   ConnectionManager. Please adjust your omnetpp.ini file accordingly");
        
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
 * @sa handleUpperMsg, handleLowerMsgStart, handleLowerMsgEnd,
 * handleSelfMsg
 */
void BasePhyLayer::handleMessage(cMessage *msg)
{
    if (msg->arrivalGateId() == uppergateIn){
        handleUpperMsg(msg);
    }
    else if(msg == txOverTimer) {
        coreEV << "transmission over" << endl;
		handleTransmissionOver();
    }
    else if (msg->isSelfMessage()) {
        if(msg->kind() == RECEPTION_COMPLETE) {
            coreEV << "frame is completely received now\n";
            // unbuffer the message
            cMessage *frame = unbufferMsg(msg);
            handleLowerMsgEnd(frame);
        }
        else {
            handleSelfMsg(msg);
        }
    }
    else {
        // msg must come from channel
        handleLowerMsgStart(msg);
        bufferMsg(msg);
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
    cMessage *timer = new cMessage(NULL, RECEPTION_COMPLETE);
    timer->setContextPointer(msg);
    scheduleAt(simTime() + (frame->getDuration()), timer);
}

/**
 * Get the context pointer to the now completely received AirFrame and
 * delete the self message
 */
cMessage *BasePhyLayer::unbufferMsg(cMessage *msg)
{
    cMessage *frame = static_cast<cMessage *>(msg->contextPointer());
    delete msg;
    return frame;
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
    frame->setPSend(transmitterPower);
    frame->setLength(headerLength);
    frame->setChannelId(channel.getActiveChannel());
    frame->encapsulate(msg);
    frame->setDuration(calcDuration(frame));
    frame->setMove(move);
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
 * Attach control info to the message and send message to the upper
 * layer. 
 *
 * @param msg AirFrame to pass to the decider
 * @param list Snr list to attach as control info
 *
 * to be called within @ref handleLowerMsgEnd.
 */
void BasePhyLayer::sendUp(cMessage *msg)
{
    send(msg, uppergateOut);
}

/**
 * send a control message to the upper layer
 */
void BasePhyLayer::sendControlUp(cMessage *msg)
{
    send(msg, upperControlOut);
}

/**
 * Convenience function which calls sendToChannel with delay set
 * to 0.0.
 *
 * It also schedules the txOverTimer which indicates the end of
 * transmission to upper layers.
 *
 * @sa sendToChannel
 */
void BasePhyLayer::sendDown(cMessage *msg)
{
    sendToChannel(msg, 0.0);
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

    assert (!txOverTimer->isScheduled());
    scheduleAt(simTime() + frame->getDuration(), txOverTimer);
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
 * If the energy of the message is high enough to really receive it
 * you should create an snr list (@ref SnrList) to be able to store
 * sn(i)r information for that message. Every time a new message
 * arrives you can add a new snr value together with a timestamp to
 * that list. Make sure to store a pointer to the mesage together with
 * the snr information to be able to retrieve it later.
 *
 * In this function also an initial SNR value can be calculated for
 * this message.
 *
 * Please take a look at SnrEval to see a "real" example.
 *
 * @sa SnrList, SnrEval
 **/
void BasePhyLayer::handleLowerMsgStart(cMessage *msg)
{

    //AirFrame *frame = static_cast<AirFrame *>(msg);

    coreEV <<"in handleLowerMsgStart"<<endl;

    //calculate the receive power

    // calculate snr information, like snr=pSend/noise or whatever....

    // if receive power is actually high enough to be able to read the
    // message and no other message is currently beeing received, store
    // the snr information for the message someweher where you can find
    // it in handleLowerMsgEnd
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This function is called right before a packet is handed on to the
 * upper layer, i.e. right after unbufferMsg. Again you can caluculate
 * some more SNR information if you want.
 *
 * You have to copy / create the SnrList related to the message and
 * pass it to sendUp() if you want to pass the message to the decider.
 *
 * Do not forget to send the message to the upper layer with sendUp()
 *
 * For a "real" implementaion take a look at SnrEval
 *
 * @sa SnrList, SnrEval
 */
void BasePhyLayer::handleLowerMsgEnd(cMessage *msg)
{
    coreEV << "in handleLowerMsgEnd\n";

    // We need to create a "dummy" snr list that we can pass together
    // with the message to the decider module so that also the
    // BasePhyLayer is able to work.
    SnrList snrList;

    // However you can take this as a reference how to create your own
    // snr entries.

    // Everytime you want to add something to the snr information list
    // it has to look like this:
    // 1. create a list entry and fill the fields
    SnrListEntry listEntry;
    listEntry.time = simTime();
    listEntry.snr = 3;          //just a senseless example

    // 2. add an entry to the SnrList
    snrList.push_back(listEntry);

    // 3. pass the message together with the list to the decider
    sendUp( decapsMsg(static_cast<AirFrame *>(msg)) );
}

/*
 * Handle completion of transmission. Override if you want to do something different.
 * Don't forget to send up the TRANSMISSION_OVER control message
 */

void BasePhyLayer::handleTransmissionOver()
{
	sendControlUp(new cMessage("TRANSMISSION_OVER", NicControlType::TRANSMISSION_OVER));
}


void BasePhyLayer::finish()
{
    if(!txOverTimer->isScheduled()) delete txOverTimer;
}

/*void BasePhyLayer::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    ChannelAccess::receiveBBItem(category, details, scopeModuleId);
    if(category == catActiveChannel) {
        channel = *(static_cast<const ActiveChannel *>(details));
    }
}*/
