/***************************************************************************
 * file:        Mac80211.cc
 *
 * author:      David Raguin / Marc Löbbers / Andreas Koepke
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
 **************************************************************************/


#include "Mac80211.h"
#include "MacControlInfo.h"
#include "PhyControlInfo.h"
#include "SimpleAddress.h"
#include <FWMath.h>

Define_Module(Mac80211);


void Mac80211::initialize(int stage)
{
    BasicLayer::initialize(stage);

    if (stage == 0)
    {
        EV << "Initializing stage 0\n";
        myMacAddr = parentModule()->id();

        switching = false;
        fsc = intrand(0x7FFFFFFF);
        if(fsc == 0) fsc = 1;
        EV << " fsc: " << fsc << "\n";

        queueLength = hasPar("queueLength") ? par("queueLength") : 10;

        radioState = RadioState::RECV;
        RadioState cs;
        Bitrate br;
        MediumIndication ind;
        
        catRadioState = bb->subscribe(this, &cs, parentModule()->id());
        catMedium = bb->subscribe(this, &ind, parentModule()->id());
        
        // timers
        timeout = new cMessage("timeout", TIMEOUT);
        nav = new cMessage("NAV", NAV);
        contention = new cMessage("contention", CONTENTION);
        endSifs = new cMessage("end SIFS", END_SIFS);
        
        state = IDLE;
        longRetryCounter = 0;
        shortRetryCounter = 0;
        rtsCtsThreshold = hasPar("rtsCtsThreshold") ? par("rtsCtsThreshold") : 1;
        
        autoBitrate = hasPar("autoBitrate") ? par("autoBitrate").boolValue() : false;
        
        delta = 1E-9;
        
        EV << "SIFS: " << SIFS << " DIFS: " << DIFS << " EIFS: " << EIFS << endl;
    }
    else if(stage == 1) {
        int channel;
        bool found = false;

        // get handle to radio
        radio = SingleChannelRadioAccess().get();
        channel = hasPar("defaultChannel") ? par("defaultChannel") : 0;
        radio->setActiveChannel(channel);

        bitrate = hasPar("bitrate") ? par("bitrate").doubleValue() : BITRATES_80211[0];
        for(int i = 0; i < 4; i++) {
            if(bitrate == BITRATES_80211[i]) {
                found = true;
                break;
            }
        }
        if(!found) bitrate = BITRATES_80211[0];
        radio->setBitrate(bitrate);
        defaultBitrate = bitrate;
        
        snrThresholds.push_back(hasPar("snr2Mbit") ? par("snr2Mbit").doubleValue() : 100);
        snrThresholds.push_back(hasPar("snr5Mbit") ? par("snr5Mbit").doubleValue() : 100);
        snrThresholds.push_back(hasPar("snr11Mbit") ? par("snr11Mbit").doubleValue() : 100);
        snrThresholds.push_back(111111111); // sentinel

        neighborhoodCacheSize = hasPar("neighborhoodCacheSize") ? par("neighborhoodCacheSize") : 0;
        neighborhoodCacheMaxAge = hasPar("neighborhoodCacheMaxAge") ? par("neighborhoodCacheMaxAge") : 10000;

        EV << " MAC Address: " << myMacAddr
           << " rtsCtsThreshold: " << rtsCtsThreshold
           << " bitrate: " << bitrate
           << " channel: " << channel
           << " autoBitrate: " << autoBitrate
           << " 2MBit: " << snrThresholds[0]
           << " 5.5MBit: " <<snrThresholds[1]
           << " 11MBit: " << snrThresholds[2]
           << " neighborhoodCacheSize " << neighborhoodCacheSize
           << " neighborhoodCacheMaxAge " << neighborhoodCacheMaxAge
           << endl;

        for(int i = 0; i < 3; i++) {
            snrThresholds[i] = FWMath::dBm2mW(snrThresholds[i]);
        }
        
        remainingBackoff = backoff();
        scheduleAt(simTime() + remainingBackoff + DIFS, contention);
    }
}

/**
 * This implementation does not support fragmentation, so it is tested
 * if the maximum length of the MPDU is exceeded.
 */
void Mac80211::handleUpperMsg(cMessage *msg)
{
    EV << "Mac80211::handleUpperMsg " << msg->name() << "\n";
    if (msg->length() > 18496)
        error("packet from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->className(), msg->name(), msg->length()/8);

    if(fromUpperLayer.size() == queueLength) {
        msg->setName("MAC ERROR");
        msg->setKind(PACKET_DROPPED);
        sendControlUp(msg);
        EV << "packet " << msg << " received from higher layer but MAC queue is full, signalling error\n";
        return;
    }

    Mac80211Pkt *mac = encapsMsg(msg);
    EV << "packet " << msg << " received from higher layer, dest=" << mac->getDestAddr() << ", encapsulated\n";

    fromUpperLayer.push_back(mac);
    // If the MAC is in the IDLE state, then start a new contention period
    if (state == IDLE && !endSifs->isScheduled()) {
        beginNewCycle();
    }
    else
    {
        EV << "enqueued, will be transmitted later\n";
    }
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all needed
 * header fields.
 */
Mac80211Pkt *Mac80211::encapsMsg(cMessage * netw)
{

    Mac80211Pkt *pkt = new Mac80211Pkt(netw->name());
    pkt->setLength(MAC80211_HEADER_LENGTH);                  // headerLength, including final CRC-field
    pkt->setRetry(false);                 // this is not a retry
    pkt->setSequenceControl(fsc++);       // add a unique fsc to it
    if(fsc <= 0) fsc = 1;
    
    // copy dest address from the Control Info attached to the network
    // mesage by the network layer
    MacControlInfo* cInfo = static_cast<MacControlInfo*>(netw->removeControlInfo());

    EV <<"CInfo removed, mac addr="<< cInfo->getNextHopMac()<<endl;
    pkt->setDestAddr(cInfo->getNextHopMac());

    //delete the control info
    delete cInfo;

    //set the src address to own mac address (nic module id())
    pkt->setSrcAddr(myMacAddr);
  
    //encapsulate the network packet
    pkt->encapsulate(netw);
    EV <<"pkt encapsulated, length: " << pkt->length() << "\n";
    
    return pkt;
}

cMessage *Mac80211::decapsMsg(Mac80211Pkt *frame) {
    cMessage *m = frame->decapsulate();
    m->setControlInfo(new MacControlInfo(frame->getSrcAddr()));
    EV << " message decapsulated " << endl;
    return m;
}

/**
 *  Handle all messages from lower layer. Checks the destination MAC
 *  adress of the packet. Then calls one of the three functions :
 *  handleMsgNotForMe(), handleBroadcastMsg(), or handleMsgForMe().
 *  Called by handleMessage().
 */
void Mac80211::handleLowerMsg(cMessage *msg)
{
    Mac80211Pkt *af = static_cast<Mac80211Pkt *>(msg);
        
    // end of the reception
    EV << "frame " << af << " received\n";
    addNeighbor(af);
    if(af->getDestAddr() == myMacAddr) {
        handleMsgForMe(af);
    }
    else if(af->getDestAddr() == L2BROADCAST) {
        handleBroadcastMsg(af);
    }
    else {
        handleMsgNotForMe(af, af->getDuration());
    }
}

void Mac80211::handleLowerControl(cMessage *msg) 
{
    EV << "Mac80211::handleLowerControl " << msg->name() << "\n";
    switch(msg->kind()) {
    case COLLISION:
    case BITERROR:
        handleMsgNotForMe(msg, EIFS);
        break;
    case TRANSMISSION_OVER:
        EV << "PHY indicated transmission over" << endl;
        radio->switchToRecv();
        radio->setBitrate(defaultBitrate);
        handleEndTransmission();
        delete msg;
        break;
    }
}


/**
 * handle timers
 */
void Mac80211::handleSelfMsg(cMessage * msg)
{
    EV << "Mac80211::handleSelfMsg " << msg->name() << "\n";
    switch (msg->kind())
    {
    case END_SIFS:
        handleEndSifsTimer();   // noch zu betrachten
        break;

    case CONTENTION:
        handleEndContentionTimer();
        break;

        // the MAC was waiting for a CTS, a DATA, or an ACK packet but the timer has expired.
    case TIMEOUT:
        handleTimeoutTimer();   // noch zu betrachten..
        break;

        // the MAC was waiting because an other communication had won the channel. This communication is now over
    case NAV:
        handleNavTimer();       // noch zu betrachten...
        break;

    default:
        error("unknown timer type");
    }
}


/**
 *  Handle all ACKs,RTS, CTS, or DATA not for the node. If RTS/CTS
 *  is used the node must stay quiet until the current handshake
 *  between the two communicating nodes is over.  This is done by
 *  scheduling the timer message nav (Network Allocation Vector).
 *  Without RTS/CTS a new contention is started. If an error
 *  occured the node must defer for EIFS.  Called by
 *  handleLowerMsg()
 */
void Mac80211::handleMsgNotForMe(cMessage *af, double duration)
{
    EV << "handle msg not for me " << af->name() << "\n";

    // if the duration of the packet is null, then do nothing (to avoid
    // the unuseful scheduling of a self message)
    if(duration != 0) {
        // the node is already deferring
        if (state == QUIET)
        {
            // the current value of the NAV is not sufficient
            if (nav->arrivalTime() < simTime() + duration)
            {
                cancelEvent(nav);
                scheduleAt(simTime() + duration, nav);
                EV << "NAV timer started for: " << duration << " State QUIET\n";
            }
        }
        // other states
        else
        {
            // if the MAC wait for another frame, it can delete its time out
            // (exchange is aborted)
            if (timeout->isScheduled()) {
                cancelEvent(timeout);
                if(state == WFACK) {
                    fromUpperLayer.front()->setRetry(true);
                }
                if((state == WFACK) || (state == WFCTS)) {
                    if(rtsCts(fromUpperLayer.front())) {
                        longRetryCounter++;
                    }
                    else {
                        shortRetryCounter++;
                    }
                }
            }
            // the node must defer for the time of the transmission
            scheduleAt(simTime() + duration, nav);
            EV << "NAV timer started, not QUIET: " << duration << endl;
            suspendContention();
            setState(QUIET);
        }
    }
    if(state == CONTEND) {
        if((af->kind() == BITERROR) || (af->kind() == COLLISION)) {
            suspendContention();
            remainingBackoff += EIFS;
        }
        else {
            beginNewCycle();
        }
    }
    delete af;
}


/**
 *  Handle a packet for the node. The result of this reception is
 *  a function of the type of the received message (RTS,CTS,DATA,
 *  or ACK), and of the current state of the MAC (WFDATA, CONTEND,
 *  IDLE, WFCTS, or WFACK). Called by handleLowerMsg()
 */
void Mac80211::handleMsgForMe(Mac80211Pkt *af)
{
    EV << "handle msg for me " << af->name() << " in " <<  stateName(state) << "\n";

    switch (state)
    {
    case IDLE:     // waiting for the end of the contention period
    case CONTEND:  // or waiting for RTS

        // RTS or DATA accepted
        if (af->kind() == RTS) {
            suspendContention();
            handleRTSframe(af);
        }
        else if (af->kind() == DATA) {
            suspendContention();
            handleDATAframe(af);
        }
        else {
            error("in handleMsgForMe() IDLE/CONTEND, strange message %s", af->name());
        }
        break;

    case WFDATA:  // waiting for DATA
        if (af->kind() == DATA) {
            handleDATAframe(af);
        }
        else {
            EV << "unexpected message -- probably a collision of RTSs\n";
            delete af;
        }
        break;

    case WFACK:  // waiting for ACK
        if (af->kind() == ACK) {
            handleACKframe(af);
        }
        else {
            error("in handleMsgForMe() WFACK, strange message %s", af->name());
        }
        delete af;
        break;

    case WFCTS:  // The MAC is waiting for CTS
        if (af->kind() == CTS) {
            handleCTSframe(af);
        }
        else {
            EV << "unexpected msg -- deleted \n";
            delete af;
        }
        break;
    case QUIET: // the node is currently deferring.

        // cannot handle any packet with its MAC adress
        delete af;
        break;

    case BUSY: // currently transmitting an ACK or a BROADCAST packet
        if(switching) {
            EV << "message received during radio state switchover\n";
            delete af;
        }
        else {
            error("logic error: node is currently transmitting, can not receive "
                  "(does the physical layer do its job correctly?)");
        }
        break;
    default:
        error("unknown state %d", state);
    }
}

/**
 *  Handle aframe wich is expected to be an RTS. Called by
 *  HandleMsgForMe()
 */
void Mac80211::handleRTSframe(Mac80211Pkt * af)
{
    if(endSifs->isScheduled()) error("Mac80211::handleRTSframe when SIFS scheduled");
    // wait a short interframe space
    endSifs->setContextPointer(af);
    scheduleAt(simTime() + SIFS, endSifs);
}


/**
 *  Handle a frame which expected to be a DATA frame. Called by
 *  HandleMsgForMe()
 */
void Mac80211::handleDATAframe(Mac80211Pkt * af)
{
    NeighborList::iterator it;
    if (rtsCts(af)) cancelEvent(timeout);  // cancel time-out event
    it = findNeighbor(af->getSrcAddr());
    if(it == neighbors.end()) error("Mac80211::handleDATAframe: neighbor not registered");
    if(af->getRetry() && (it->fsc == af->getSequenceControl())) {
        EV << "Mac80211::handleDATAframe suppressed duplicate message " << af
           << " fsc: " << it->fsc << "\n";
    }
    else {
        it->fsc = af->getSequenceControl();
        // pass the packet to the upper layer
        sendUp(decapsMsg(af));
    }
    // wait a short interframe space
    if(endSifs->isScheduled()) error("Mac80211::handleDATAframe when SIFS scheduled");
    endSifs->setContextPointer(af);
    scheduleAt(simTime() + SIFS, endSifs);
}


/**
 *  Handle ACK and delete corresponding packet from queue
 */
void Mac80211::handleACKframe(Mac80211Pkt * af)
{
    // cancel time-out event
    cancelEvent(timeout);

    // the transmission is acknowledged : initialize long_retry_counter
    longRetryCounter = 0;
    shortRetryCounter = 0;
    remainingBackoff = backoff();
    // removes the acknowledged packet from the queue
    Mac80211Pkt *temp = fromUpperLayer.front();
    fromUpperLayer.pop_front();
    delete temp;

    // if thre's a packet to send and if the channel is free then start a new contention period
    beginNewCycle();
}


/**
 *  Handle a CTS frame. Called by HandleMsgForMe(Mac80211Pkt* af)
 */
void Mac80211::handleCTSframe(Mac80211Pkt * af)
{
    // cancel time-out event
    cancelEvent(timeout);
    shortRetryCounter = 0;
    // wait a short interframe space
    if(endSifs->isScheduled()) error("Mac80211::handleCTSframe when SIFS scheduled");
    endSifs->setContextPointer(af);
    scheduleAt(simTime() + SIFS, endSifs);
}


/**
 *  Handle a broadcast packet. This packet is simply passed to the
 *  upper layer. No acknowledgement is needed.  Called by
 *  handleLowerMsg(Mac80211Pkt *af)
 */
void Mac80211::handleBroadcastMsg(Mac80211Pkt *af)
{
    EV << "handle broadcast\n";
    if((state == BUSY) && (!switching)) {
        error("logic error: node is currently transmitting, can not receive "
              "(does the physical layer do its job correctly?)");
    }
    sendUp(decapsMsg(af));
    delete af;
    if (state == CONTEND) {
        suspendContention();
        beginNewCycle();
    }
}


/**
 *  The node has won the contention, and is now allowed to send an
 *  RTS/DATA or Broadcast packet. The backoff value is deleted and
 *  will be newly computed in the next contention period
 */
void Mac80211::handleEndContentionTimer()
{
    if(state == IDLE) {
        remainingBackoff = 0;
    }
    else if(state == CONTEND) {
        // the node has won the channel, the backoff window is deleted and
        // will be new calculated in the next contention period
        remainingBackoff = 0;
        // unicast packet
        radio->switchToSend();
        if (!nextIsBroadcast)
        {
            if(rtsCts(fromUpperLayer.front())) {
                // send a RTS
                sendRTSframe();
            }
            else { 
                sendDATAframe(0);
            }
        }// broadcast packet
        else {
            sendBROADCASTframe();
            // removes the packet from the queue without waiting for an acknowledgement
            Mac80211Pkt *temp = fromUpperLayer.front();
            fromUpperLayer.pop_front();
            delete(temp);
        }
    }
    else {
        error("logic error: expiration of the contention timer outside of CONTEND/IDLE state, should not happen");
    }
}

/**
 *  Handle the NAV timer (end of a defering period). Called by
 *  HandleTimer(cMessage* msg)
 */
void Mac80211::handleNavTimer()
{
    if (state != QUIET)
        error("logic error: expiration of the NAV timer outside of the state QUIET, should not happen");

    // if there's a packet to send and if the channel is free, then start a new contention period
    beginNewCycle();
}


/**
 *  Handle the time out timer. Called by handleTimer(cMessage* msg)
 */
void Mac80211::handleTimeoutTimer()
{
    EV << "Mac80211::handleTimeoutTimer " << stateName(state) << "\n";
    if(state == WFCTS) {
        longRetryCounter++;
        remainingBackoff = backoff();
    }
    else if(state == WFACK) {
        if(rtsCts(fromUpperLayer.front())) {
            longRetryCounter++;
            fromUpperLayer.front()->setRetry(true);
        }
        else {
            shortRetryCounter++;
            fromUpperLayer.front()->setRetry(true);
        }
        remainingBackoff = backoff();
    }
    // if there's a packet to send and if the channel is free then
    // start a new contention period
    if (state != QUIET)
        beginNewCycle();
}


/**
 *  Handle the end sifs timer. Then sends a CTS, a DATA, or an ACK
 *  frame
 */
void Mac80211::handleEndSifsTimer()
{
    Mac80211Pkt *frame = static_cast<Mac80211Pkt *>(endSifs->contextPointer());
    radio->switchToSend();
    switch (frame->kind())
    {
    case RTS:
        sendCTSframe(frame);
        break;
    case CTS:
        sendDATAframe(frame);
        break;
    case DATA:
        sendACKframe(frame);
        break;
    default:
        error("logic error: end sifs timer when previously received packet is not RTS/CTS/DATA");
    }

    // don't need previous frame any more
    delete frame;
}

/**
 *  Handle the end of transmission timer (end of the transmission
 *  of an ACK or a broadcast packet). Called by
 *  HandleTimer(cMessage* msg)
 */
void Mac80211::handleEndTransmission()
{
    EV << "transmission of packet is over\n";
    if(state == BUSY) {
        if(nextIsBroadcast) {
            shortRetryCounter = 0;
            longRetryCounter = 0;
            remainingBackoff = backoff();
        }
        beginNewCycle();
    }
}

/**
 *  Send a DATA frame. Called by HandleEndSifsTimer() or
 *  handleEndContentionTimer()
 */
void Mac80211::sendDATAframe(Mac80211Pkt *af)
{
    Mac80211Pkt *frame = static_cast<Mac80211Pkt *>(fromUpperLayer.front()->dup());
    double br;
    PhyControlInfo *pco;
    
    if(af) {
        pco = static_cast<PhyControlInfo*>(af->removeControlInfo());
        br = pco->getBitrate();
    }
    else {
       br  = retrieveBitrate(frame->getDestAddr());
       pco = new PhyControlInfo(br);
       if(shortRetryCounter) frame->setRetry(true);
    }
    // build a copy of the frame in front of the queue'
    frame->setControlInfo(pco);
    frame->setSrcAddr(myMacAddr);
    frame->setKind(DATA);
        
    radio->setBitrate(br);

    if (rtsCts(frame)) {
        frame->setDuration(SIFS + packetDuration(LENGTH_ACK, br));
        frame->setKind(DATA);
    }
    else {
        frame->setDuration(0);
    }

    // schedule time out
    scheduleAt(simTime() + timeOut(DATA, br), timeout);
    EV << "sending DATA  to " << frame->getDestAddr() << " with bitrate " << br << endl;
    // send DATA frame
    sendDown(frame);

    // update state and display
    setState(WFACK);
}


/**
 *  Send an ACK frame.Called by HandleEndSifsTimer()
 */
void Mac80211::sendACKframe(Mac80211Pkt * af)
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-ack");

    PhyControlInfo *pco = static_cast<PhyControlInfo*>(af->removeControlInfo());
    double br = pco->getBitrate();
    
    frame->setControlInfo(pco);
    frame->setKind(ACK);
    frame->setLength((int)LENGTH_ACK);

    // the dest address must be the src adress of the RTS or the DATA
    // packet received. The src adress is the adress of the node
    frame->setSrcAddr(myMacAddr);
    frame->setDestAddr(af->getSrcAddr());
    frame->setDuration(0);

    radio->setBitrate(br);
    
    sendDown(frame);
    EV << "sent ACK frame!\n";

    // update state and display
    setState(BUSY);
}


/**
 *  Send a RTS frame.Called by handleContentionTimer()
 */
void Mac80211::sendRTSframe()
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-rts");
    double br = retrieveBitrate(fromUpperLayer.front()->getDestAddr());
    
    PhyControlInfo *pco = new PhyControlInfo(br);
    
    frame->setControlInfo(pco);

    frame->setKind(RTS);
    frame->setLength((int)LENGTH_RTS);

    // the src adress and dest address are copied in the frame in the queue (frame to be sent)
    frame->setSrcAddr(fromUpperLayer.front()->getSrcAddr());
    frame->setDestAddr(fromUpperLayer.front()->getDestAddr());
    frame->setDuration(3 * SIFS + packetDuration(LENGTH_CTS, br) +
                       packetDuration(fromUpperLayer.front()->length(), br) +
                       packetDuration(LENGTH_ACK, br));
    EV << " Mac80211::sendRTSframe duration: " <<  packetDuration(LENGTH_RTS, br) << " br: " << br << "\n";
    radio->setBitrate(br);
    
    // schedule time-out
    scheduleAt(simTime() + timeOut(RTS, br), timeout);

    // send RTS frame
    sendDown(frame);

    // update state and display
    setState(WFCTS);
}


/**
 *  Send a CTS frame.Called by HandleEndSifsTimer()
 */
void Mac80211::sendCTSframe(Mac80211Pkt * af)
{
    double br;
    
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-cts");
    
    PhyControlInfo *pco = static_cast<PhyControlInfo*>(af->removeControlInfo());
    br = pco->getBitrate();
    
    frame->setControlInfo(pco);
    frame->setKind(CTS);
    frame->setLength((int)LENGTH_CTS);

    // the dest adress must be the src adress of the RTS received. The
    // src adress is the adress of the node
    frame->setSrcAddr(myMacAddr);
    frame->setDestAddr(af->getSrcAddr());
    frame->setDuration(af->getDuration() - SIFS - packetDuration(LENGTH_CTS, br));

    radio->setBitrate(br);
    
    scheduleAt(simTime() + af->getDuration() - packetDuration(LENGTH_ACK, br) - 2 * SIFS + delta, timeout);
    EV << " Mac80211::sendCTSframe duration: " <<  packetDuration(LENGTH_CTS, br) << " br: " << br << "\n";
    // send CTS frame
    sendDown(frame);

    // update state and display
    setState(WFDATA);
}

/**
 *  Send a BROADCAST frame.Called by handleContentionTimer()
 */
void Mac80211::sendBROADCASTframe()
{
    // send a copy of the frame in front of the queue
    Mac80211Pkt *frame = static_cast<Mac80211Pkt *>(fromUpperLayer.front()->dup());
    double br = retrieveBitrate(frame->getDestAddr());
    PhyControlInfo *pco = new PhyControlInfo(br);
    frame->setControlInfo(pco);
    frame->setKind(BROADCAST);
    radio->setBitrate(br);
    
    sendDown(frame);
    // update state and display
    setState(BUSY);
}

/**
 *  Start a new contention period if the channel is free and if
 *  there's a packet to send.  Called at the end of a deferring
 *  period, a busy period, or after a failure. Called by the
 *  HandleMsgForMe(), HandleTimer() HandleUpperMsg(), and, without
 *  RTS/CTS, by handleMsgNotForMe().
 */
void Mac80211::beginNewCycle()
{
    // before trying to send one more time a packet, test if the
    // maximum retry limit is reached. If it is the case, then
    // delete the packet and send the next packet.
    testMaxAttempts();

    if (!fromUpperLayer.empty()) {

        // look if the next packet is unicast or broadcast
        nextIsBroadcast = (fromUpperLayer.front()->getDestAddr() == L2BROADCAST);

        setState(CONTEND);
        if(!contention->isScheduled()) {
            if(medium == MediumIndication::IDLE) {
                scheduleAt(simTime() + DIFS + remainingBackoff, contention);
            }
        }
    }
    else {
        if(!contention->isScheduled()) {
            scheduleAt(simTime() + DIFS + remainingBackoff, contention);
        }
        setState(IDLE);
    }
}

/**
 * Compute the backoff value.
 */
double Mac80211::backoff() {
    return ((double) intrand(contentionWindow() + 1)) * ST;
}

/**
 *  Compute the contention window with the binary backoff
 *  algorithm.
 */
int Mac80211::contentionWindow()
{
    unsigned cw = ((CW_MIN + 1) << longRetryCounter) - 1;
    if(cw > CW_MAX) cw = CW_MAX;
    return cw;
}

/**
 *  Test if the maximal retry limit is reached, and delete the
 *  frame to send in this case.
 */
void Mac80211::testMaxAttempts()
{
    if ((longRetryCounter >= LONG_RETRY_LIMIT) || (shortRetryCounter >= SHORT_RETRY_LIMIT)) {
        EV << "retry limit reached src: "<< shortRetryCounter << " lrc: " << longRetryCounter << endl;
        // initialize counter
        longRetryCounter = 0;
        shortRetryCounter = 0;
        // delete the frame to transmit
        Mac80211Pkt *temp = fromUpperLayer.front();
        fromUpperLayer.pop_front();
        temp->setName("MAC ERROR");
        temp->setKind(PACKET_DROPPED);
        sendControlUp(temp);
    }
}

/**
 * Handle change nofitications. In this layer it is usually
 * information about the radio channel, i.e. if it is IDLE etc.
 */
void Mac80211::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    Enter_Method_Silent();
    BasicLayer::receiveBBItem(category, details, scopeModuleId);
    EV << "Mac80211::receiveBBItem " << details->info() << "\n";
    if(category == catRadioState) {
        radioState = static_cast<const RadioState *>(details)->getState();
        switch(radioState)
        {
        case RadioState::SWITCH_TO_SLEEP:
        case RadioState::SWITCH_TO_RECV:
        case RadioState::SWITCH_TO_SEND:
            switching = true;
            break;
        case RadioState::SLEEP:
        case RadioState::RECV:
        case RadioState::SEND:
            switching = false;
            break;
        default:
            error("Mac80211::receiveBBItem unknown radio state");
            break;
        }
    }
    else if(category == catMedium) {
        medium = static_cast<const MediumIndication *>(details)->getState();
        // beginning of a reception
        if(medium == MediumIndication::BUSY)
        {   
            EV << "medium busy" << endl;
            if(state != IDLE) suspendContention();

            // if there's a SIFS period
            if (endSifs->isScheduled())
            {
                // delete the previously received frame
                delete static_cast<Mac80211Pkt *>(endSifs->contextPointer());

                // cancel the next transmission
                cancelEvent(endSifs);

                // state in now IDLE or CONTEND
                if (fromUpperLayer.empty())
                    setState(IDLE);
                else
                    setState(CONTEND);
            }
        }
        else {
            EV << "medium idle" << endl;
        }
    }
}


/**
 *  Return a time-out value for a type of frame. Called by
 *  SendRTSframe, sendCTSframe, etc.
 */
double Mac80211::timeOut(frameType_802_11 type, double br)
{
    double time_out = 0;
    
    switch (type)
    {
    case RTS:
        time_out = SIFS + packetDuration(LENGTH_RTS, br) + packetDuration(LENGTH_CTS, br) + delta;
        EV << " Mac80211::timeOut RTS " << time_out << "\n";
        break;
    case DATA:
        time_out = SIFS + packetDuration(fromUpperLayer.front()->length(), br) + packetDuration(LENGTH_ACK, br) + delta;
        EV << " Mac80211::timeOut DATA " << time_out << "\n";
        break;
    default:
        EV << "Unused frame type was given when calling timeOut(), this should not happen!\n";
    }
    return time_out;
}

/**
 * Computes the duration of the transmission of a frame over the
 * physical channel. 'bits' should be the total length of the MAC
 * packet in bits.
 */
double Mac80211::packetDuration(double bits, double br)
{
    return bits / br + PHY_HEADER_LENGTH / BITRATE_HEADER;
}

const char *Mac80211::stateName(State state)
{
#define CASE(x) case x: s=#x; break
    const char *s = "???";
    switch (state)
    {
        CASE(WFDATA);
        CASE(QUIET);
        CASE(IDLE);
        CASE(CONTEND);
        CASE(WFCTS);
        CASE(WFACK);
        CASE(BUSY);
    }
    return s;
#undef CASE
}

void Mac80211::setState(State newState)
{
    if (state==newState)
        EV << "staying in state " << stateName(state) << "\n";
    else
        EV << "state " << stateName(state) << " --> " << stateName(newState) << "\n";
    state = newState;
}

void Mac80211::suspendContention()  {
    // if there's a contention period
    if(contention->isScheduled()) {
        // update the backoff window in order to give higher priority in
        // the next battle
        if(simTime() - contention->sendingTime() < DIFS) {
            remainingBackoff = (contention->arrivalTime() - contention->sendingTime() - DIFS);
        }
        else {
            remainingBackoff = ((int)((contention->arrivalTime() - simTime() - DIFS)/ST)) * ST;
        }
        EV << "suspended backoff timer, remaining backoff time: " << remainingBackoff << endl;
        cancelEvent(contention);
    }
    else {
        if(remainingBackoff == 0) {
            remainingBackoff = backoff();
            EV << " some event happend that needs a new backoff: "  << remainingBackoff << endl;
        }
    }
}

double Mac80211::retrieveBitrate(int destAddress) {
    double bitrate = defaultBitrate;
    NeighborList::iterator it;
    if(autoBitrate && (destAddress != L2BROADCAST) &&
       (longRetryCounter == 0) && (shortRetryCounter == 0)) {
        it = findNeighbor(destAddress);
        if((it != neighbors.end()) && (it->age > (simTime() - neighborhoodCacheMaxAge))) {
            bitrate = it->bitrate;
        }
    }
    return bitrate;
}

void Mac80211::addNeighbor(Mac80211Pkt *af) {
    int srcAddress = af->getSrcAddr();
    NeighborList::iterator it = findNeighbor(srcAddress);
    double snr = static_cast<PhyControlInfo *>(af->controlInfo())->getSnr();
    double bitrate = BITRATES_80211[0];
    NeighborEntry entry;
    
    if(snr > snrThresholds[0]) bitrate = BITRATES_80211[1];
    if(snr > snrThresholds[1]) bitrate = BITRATES_80211[2];
    if(snr > snrThresholds[2]) bitrate = BITRATES_80211[3];

    if(it != neighbors.end()) {
        it->age = simTime();
        it->bitrate = bitrate;
    }
    else {
        if(neighbors.size() < neighborhoodCacheSize) {
            entry.address = srcAddress;
            entry.age = simTime();
            entry.bitrate = bitrate;
            entry.fsc = 0;
            neighbors.push_back(entry);
        }
        else {
            it = findOldestNeighbor();
            if(it != neighbors.end()) {
                it->age = simTime();
                it->bitrate = bitrate;
                it->address = srcAddress;
                it->fsc = 0;
            }
        }
    }
    EV << "updated information for neighbor: " << srcAddress
       << " snr: " << snr << " bitrate: " << bitrate << endl;
}

void Mac80211::finish() {
    BasicLayer::finish();
    MacPktList::iterator it;
    
    if(!timeout->isScheduled()) delete timeout;
    if(!nav->isScheduled()) delete nav;
    if(!contention->isScheduled()) delete contention;
    if(!endSifs->isScheduled()) delete endSifs;
    
    for(it = fromUpperLayer.begin(); it != fromUpperLayer.end(); ++it) {
        delete (*it);
    }
    fromUpperLayer.clear();
}

