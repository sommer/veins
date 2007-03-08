/***************************************************************************
 * file:        BasicSnrEval.cc
 *
 * author:      Marc Loebbers, Jochen Adamek
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


#include "BasicSnrEval.h"

#include "ChannelControl.h"
#include "CoreDebug.h"

const double BasicSnrEval::speedOfLight = ChannelControl::speedOfLight;

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
void BasicSnrEval::initialize(int stage)
{
    ChannelAccess::initialize(stage);

    if (stage == 0){
        uppergateIn = findGate("uppergateIn");
        uppergateOut = findGate("uppergateOut");
        upperControlOut = findGate("upperControlOut");
        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;
        headerLength = par("headerLength");

        hasPar("transmitterPower") ? transmitterPower=par("transmitterPower").doubleValue() :
            transmitterPower = static_cast<double>(cc->par("pMax"));

        hasPar("sensitivity") ? sensitivity=FWMath::dBm2mW(par("sensitivity").doubleValue()) :
            sensitivity = FWMath::dBm2mW(static_cast<double>(cc->par("sat")));

        hasPar("carrierFrequency") ? carrierFrequency=par("carrierFrequency").doubleValue() :
            carrierFrequency = static_cast<double>(cc->par("carrierFrequency"));

        hasPar("alpha") ? alpha=par("alpha").doubleValue() :
            alpha = static_cast<double>(cc->par("alpha"));
        

        catActiveChannel = bb->subscribe(this, &channel, parentModule()->id());
    }
    else {
        if(alpha < cc->par("alpha").doubleValue())
            error("SnrEval::initialize() alpha can't be smaller than in \
                   ChannelControl. Please adjust your omnetpp.ini file accordingly");
        
        if(transmitterPower > cc->par("pMax").doubleValue())
            error("SnrEval::initialize() tranmitterPower can't be bigger than \
                   pMax in ChannelControl! Please adjust your omnetpp.ini file accordingly");
        
        if(sensitivity < FWMath::dBm2mW(cc->par("sat").doubleValue()))
            error("SnrEval::initialize() sensitivity can't be smaller than the signal attentuation threshold (sat) in \
                   ChannelControl. Please adjust your omnetpp.ini file accordingly");
        
        if(carrierFrequency < cc->par("carrierFrequency").doubleValue())
            error("SnrEval::initialize() carrierFrequency can't be smaller than in \
                   ChannelControl. Please adjust your omnetpp.ini file accordingly");
        
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
void BasicSnrEval::handleMessage(cMessage *msg)
{
    if (msg->arrivalGateId() == uppergateIn){
        AirFrame *frame = encapsMsg(msg);
        handleUpperMsg(frame);
    }
    else if(msg == txOverTimer) {
        coreEV << "transmission over" << endl;
        sendControlUp(new cMessage("TRANSMISSION_OVER", NicControlType::TRANSMISSION_OVER));
    }
    else if (msg->isSelfMessage()) {
        if(msg->kind() == RECEPTION_COMPLETE) {
            coreEV << "frame is completely received now\n";
            // unbuffer the message
            AirFrame *frame = unbufferMsg(msg);
            handleLowerMsgEnd(frame);    
            delete msg;
        }
        else if(msg->kind() == DELAY_OVER) {
        	coreEV << "delay is finished\n";
        	AirFrame *frame = static_cast<AirFrame *>(msg->contextPointer());
        	//buffer the message
        	bufferMsg(frame);
        	handleLowerMsgStart(frame);
        	//change the kind-field to RECEPTION_COMPLETE
        	msg->setKind(RECEPTION_COMPLETE);        
        	scheduleAt(simTime() + (frame->getDuration()), msg);	
        }
        else {
            handleSelfMsg(msg);
            
        }
        
    }
    else {
        // msg must come from channel
        AirFrame *frame = static_cast<AirFrame *>(msg);
        cMessage *timer = new cMessage(NULL,DELAY_OVER);
        timer->setContextPointer(frame);
        scheduleAt(simTime() + calcDelay(frame), timer);
    }
}

/**
 * The packet is put in a buffer for the time the transmission would
 * last in reality. A timer indicates when the transmission is
 * complete. So, look at unbufferMsg to see what happens when the
 * transmission is complete..
 */
void BasicSnrEval::bufferMsg(AirFrame * frame)
{
    // set timer to indicate transmission is complete
    cMessage *timer = new cMessage(NULL, RECEPTION_COMPLETE);
    timer->setContextPointer(frame);
    scheduleAt(simTime() + (frame->getDuration()), timer);
}

/**
 * Get the context pointer to the now completely received AirFrame and
 * delete the self message
 */
AirFrame *BasicSnrEval::unbufferMsg(cMessage *msg)
{
    AirFrame *frame = static_cast<AirFrame *>(msg->contextPointer());
    delete msg;
    return frame;
}

/**
 * This function encapsulates messages from the upper layer into an
 * AirFrame, copies the type and channel fields, adds the
 * headerLength, sets the pSend (transmitterPower) and returns the
 * AirFrame.
 */
AirFrame *BasicSnrEval::encapsMsg(cMessage *msg)
{
    AirFrame *frame = new AirFrame(msg->name(), msg->kind());
    frame->setPSend(transmitterPower);
    frame->setLength(headerLength);
    frame->setChannelId(channel.getActiveChannel());
    frame->encapsulate(msg);
    frame->setDuration(calcDuration(frame));
    frame->setHostMove(hostMove);
    //frame->setDistance(sqrDistance);
    return frame;
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
void BasicSnrEval::sendUp(AirFrame *msg, const SnrList& list)
{
    // create ControlInfo
    SnrControlInfo *cInfo = new SnrControlInfo;
    // attach the list to cInfo
    cInfo->setSnrList(list);
    // attach the cInfo to the AirFrame
    msg->setControlInfo(cInfo);
    send(static_cast<cMessage *>(msg), uppergateOut);
}

/**
 * send a control message to the upper layer
 */
void BasicSnrEval::sendControlUp(cMessage *msg)
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
void BasicSnrEval::sendDown(AirFrame *msg)
{
    sendToChannel(static_cast<cMessage *>(msg), 0.0);
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to the channel.
 *
 * The MAC frame is already encapsulated in an AirFrame and all standard
 * header fields are set.
 */
void BasicSnrEval::handleUpperMsg(AirFrame * frame)
{
    scheduleAt(simTime() + (frame->getDuration()), txOverTimer);
    sendDown(frame);
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This function is called right before a packet is handed on to the
 * upper layer, i.e. right after unbufferMsg. Again you can calculate
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
void BasicSnrEval::handleLowerMsgEnd(AirFrame * frame)
{
    coreEV << "in handleLowerMsgEnd\n";

    // We need to create a "dummy" snr list that we can pass together
    // with the message to the decider module so that also the
    // BasicSnrEval is able to work.
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
    sendUp(frame, snrList);
}


void BasicSnrEval::finish()
{
    if(!txOverTimer->isScheduled()) delete txOverTimer;
}

void BasicSnrEval::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    ChannelAccess::receiveBBItem(category, details, scopeModuleId);
    if(category == catActiveChannel) {
        channel = *(static_cast<const ActiveChannel *>(details));
    }
}
//Neu
double BasicSnrEval::calcDelay(AirFrame *frame){
double delay = 0.0;
double dist = frame->getDistance();
//delay = calcSqrdistance(frame)/speedOfLight;
delay = dist/speedOfLight;
return delay;
}

double BasicSnrEval::calcSqrdistance(AirFrame *frame){
	Coord myPos(hostMove.startPos);
	HostMove rHm(frame->getHostMove());
	Coord framePos(rHm.startPos);
	double sqrdistance = 0.0;
	
	//calculate distance
    if(useTorus) {
        sqrdistance = myPos.sqrTorusDist(framePos, playground);
        frame->setDistance(sqrdistance);
    } else {
        sqrdistance = myPos.sqrdist(framePos);
        frame->setDistance(sqrdistance);
    }
       
       
    coreEV << " sqrdistance: "<< sqrt(sqrdistance) << " Torus: "<<useTorus<< endl;
    
    return sqrdistance;
}

    

