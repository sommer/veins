/***************************************************************************
 * file:        Mac80211a.cc
 *
 * author:      Thomas Freitag, David Raguin / Marc Löbbers / Andreas Koepke
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


#include "Mac80211a.h"
#include "MacControlInfo.h"
#include "PhyControlInfo.h"
#include "SimpleAddress.h"
#include "RSSI.h"
#include <FWMath.h>
#include <OFDMASpecials.h>
#include <SinkSpecials.h>

Define_Module(Mac80211a);

#define RECORD_PACKET_ERROR(l)		recordToSink(RECORD_PACKET_ERROR,l)
#define RECORD_PACKET_SUCCESS(l)	recordToSink(RECORD_PACKET_SUCCESS,l)
#define RECORD_PACKET_NUMFRAG(l)	recordToSink(RECORD_PACKET_NUMFRAG,l)
#define RECORD_PACKET_DELAY(d)		recordToSink(RECORD_PACKET_DELAY,d)
#define RECORD_PACKET_RATE(d)		recordToSink(RECORD_PACKET_RATE,d)
#define RECORD_CONTROL_FRAME_ERROR(l)	recordToSink(RECORD_CONTROL_FRAME_ERROR,l)
#define RECORD_CONTROL_FRAME_SUCCESS(l)	recordToSink(RECORD_CONTROL_FRAME_SUCCESS,l)
#define RECORD_DATA_FRAME_ERROR(l)	recordToSink(RECORD_DATA_FRAME_ERROR,l)
#define RECORD_DATA_FRAME_SUCCESS(l)	recordToSink(RECORD_DATA_FRAME_SUCCESS,l)
#define RECORD_DATA_FRAME_RATE(l)	recordToSink(RECORD_DATA_FRAME_RATE,l)
#define RECORD_TOTAL_FRAME_RATE(l)	recordToSink(RECORD_TOTAL_FRAME_RATE,l)
#define RECORD_SELECTED_MODE(i,l)	recordToSink(RECORD_SELECTED_MODE,i,l)
#define RECORD_DESTNUM(l)		recordToSink(RECORD_DESTNUM,l)
#define RECORD_TIMEOVERHEAD_RATE(d)	recordToSink(RECORD_TIMEOVERHEAD_RATE,d)
#define RECORD_BITOVERHEAD_RATE(d)	recordToSink(RECORD_BITOVERHEAD_RATE,d)
#define RECORD_FRAGMENTLENGTH(l)	recordToSink(RECORD_FRAGMENTLENGTH,l)


void Mac80211a::initialize(int stage)
{
    BasicLayer::initialize(stage);

    if (stage == 0)
    {
        myMacAddr = parentModule()->id();
	
	currentControlDelay = 0;

        EV << "Initializing stage 0\n";
        queueLength = hasPar("queueLength") ? par("queueLength") : 10;

        busyRSSI = hasPar("busyRSSI") ? par("busyRSSI") : -132;
	busyRSSI = FWMath::dBm2mW(busyRSSI);

        radioState = RadioState::RECV;
        rssi = 0;
        RadioState cs;
        RSSI rs;
        Bitrate br;

	toAssignment = findGate("toAssignment");
	fromAssignment = findGate("fromAssignment");
	toSink = findGate("toSink");

	bufState = new BBBufferState(EMPTY);
        
        catRadioState = bb->subscribe(this, &cs, parentModule()->id());
        catRSSI = bb->subscribe(this, &rs, parentModule()->id());
        
        // timers
        timeout = new cMessage("timeout", TIMEOUT);
        nav = new cMessage("NAV", NAV);
        contention = new cMessage("contention", CONTENTION);
        endSifs = new cMessage("end SIFS", END_SIFS);

	extendedCycle = false;
	noDelayedAckScheduled = true;
        
        state = IDLE;

	noeCtsScheduled=true;

        longRetryCounter = 0;
        shortRetryCounter = 0;

	overheadBitCounter=0;
	payloadBitCounter=0;

	overheadTimeCounter=-1.0;
	payloadTimeCounter=-1.0;

	tempOverheadTimeCounter=-1.0;
	tempPayloadTimeCounter=-1.0;

	// additions for fragmentation by Thomas Freitag
	//maxFragmentNumber = 0;
	//curFragmentNumber = 0;
	sequenceNumber = myMacAddr; // this is a dirty hack to avoid equal sequence numbers // TF
	fragmentationThreshold = hasPar("fragmentationThreshold") ? par("fragmentationThreshold") : 18496;
	defragBufferIsComplete = false;
	
	WATCH(fragmentationThreshold);
	
	clearDefragBuffer();
	
        rtsCtsThreshold = hasPar("rtsCtsThreshold") ? par("rtsCtsThreshold") : 1;

	snrThresholds.push_back(FWMath::dBm2mW(hasPar("ThresholdMode0") ? par("ThresholdMode0") : 3.1730));
	snrThresholds.push_back(FWMath::dBm2mW(hasPar("ThresholdMode1") ? par("ThresholdMode1") : 4.3730));
	snrThresholds.push_back(FWMath::dBm2mW(hasPar("ThresholdMode2") ? par("ThresholdMode2") : 6.1640));
	snrThresholds.push_back(FWMath::dBm2mW(hasPar("ThresholdMode3") ? par("ThresholdMode3") : 7.3640));
	snrThresholds.push_back(FWMath::dBm2mW(hasPar("ThresholdMode4") ? par("ThresholdMode4") : 13.0468));
	snrThresholds.push_back(FWMath::dBm2mW(hasPar("ThresholdMode5") ? par("ThresholdMode5") : 14.2468));
	snrThresholds.push_back(FWMath::dBm2mW(hasPar("ThresholdMode6") ? par("ThresholdMode6") : 20.0730));
	snrThresholds.push_back(FWMath::dBm2mW(hasPar("ThresholdMode7") ? par("ThresholdMode7") : 20.3730));
        
        autoBitrate = hasPar("autoBitrate") ? par("autoBitrate") : false;

        useOFDMAextensions = hasPar("useOFDMAextensions") ? par("useOFDMAextensions") : false;
	
	controlBitrate = BITRATES_80211a[0];
	currentBitrate = controlBitrate;

	ackCtsDuration = packetDuration(LENGTH_CTS,controlBitrate);
	eCtsDuration = packetDuration(LENGTH_ECTS,controlBitrate);

	WATCH(controlBitrate);
	WATCH(ackCtsDuration);
	WATCH(eCtsDuration);
        
        delta = 1E-9;
        
        EV << "SIFS: " << SIFS << " DIFS: " << DIFS << " EIFS: " << EIFS << endl;
	EV << "ACK length: "<<ackCtsDuration<<" control frame bitrate: "<<controlBitrate<<endl;
    }
    else if(stage == 1) {
        int channel;
        bool found = false;

        // get handle to radio
        radio = SingleChannelRadioAccess().get();
        channel = hasPar("defaultChannel") ? par("defaultChannel") : 0;
        radio->setActiveChannel(channel);

        bitrate = hasPar("bitrate") ? par("bitrate") : BITRATES_80211a[0];
        for(int i = 0; i < 7; i++) {
            if(bitrate == BITRATES_80211a[i]) {
                found = true;
                break;
            }
        }
        if(!found) bitrate = BITRATES_80211a[0];
        radio->setBitrate(bitrate);
        defaultBitrate = bitrate;
        
        neighborhoodCacheSize = hasPar("neighborhoodCacheSize") ? par("neighborhoodCacheSize") : 0;
        neighborhoodCacheMaxAge = hasPar("neighborhoodCacheMaxAge") ? par("neighborhoodCacheMaxAge") : 10000;

        EV << " MAC Address: " << myMacAddr
           << " rtsCtsThreshold: " << rtsCtsThreshold
           << " bitrate: " << bitrate
           << " busyRSSI  " << busyRSSI
           << " channel: " << channel
           << " autoBitrate: " << autoBitrate
           << " neighborhoodCacheSize " << neighborhoodCacheSize
           << " neighborhoodCacheMaxAge " << neighborhoodCacheMaxAge
           << endl;

        //for(int i = 0; i < 3; i++) {
        //    snrThresholds[i] = FWMath::dBm2mW(snrThresholds[i]);
        //    EV << " after conversion to ratio: " << snrThresholds[i] << endl;
        //}
        //busyRSSI = FWMath::dBm2mW(busyRSSI);
        remainingBackoff = backoff();

	bufStateCat = bb->getCategory(bufState);
	bb->publishBBItem(bufStateCat, bufState, parentModule()->parentModule()->id());
    }
}


void Mac80211a::handleMessage(cMessage *msg) {
    // handles additional messages from assignment module
    if (msg->arrivalGateId()==fromAssignment)
        handleAssignmentMsg(msg);
    else
    	BasicLayer::handleMessage(msg);
}


/*
 * FYI:
 *
 * enum list_actions {
 *      ADD_TO_MODELIST,        // add modes
 *      CLEAR_MODELIST,         // clear mode list
 *      REMOVE_FROM_MODELIST,   // remove modes for given station
 *      CALC_ASSIGNMENT,        // calculate assignment
 *      ASSIGNMENT_REPLY        // calculated assignment
 * };
 */
void Mac80211a::handleAssignmentMsg(cMessage *msg) {
    BasicAssignmentMessage* aMsg = static_cast<BasicAssignmentMessage*>(msg); 
    // message handling
    if (aMsg->kind()==ASSIGNMENT_REPLY) {
    	myAssignment = aMsg->getAssignment();
	EV<<"Assignment received."<<endl;
    }
    else
        error("Unexpected message kind");
    delete aMsg;
}

void Mac80211a::handleUpperMsg(cMessage *msg)
{
    // fragmentation is supported now // TF
    /* 
    if (msg->length() > 18496)
        error("packet from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              msg->className(), msg->name(), msg->length()/8);
    */

    // added for fragmentation support // TF
    if (msg->length() > fragmentationThreshold*16) {
    	msg->setName("MAC ERROR");
	msg->setKind(PACKET_DROPPED);
	sendControlUp(msg);
	EV << "packet" << msg << " to long: "<<msg->length() << ", signaling error" << endl;
	return;
    }

    if (fromUpperLayer.size() == queueLength) {
        msg->setName("MAC ERROR");
        msg->setKind(PACKET_DROPPED);
        sendControlUp(msg);
	bufState->setBufferState(FULL);
        EV << "packet " << msg << " received from higher layer but MAC queue is full, signalling error\n";
        return;
    }

    if (fromUpperLayer.size()/queueLength < 1/4)
    	bufState->setBufferState(LOW);
    else 
    	bufState->setBufferState(HIGH);
    
    // make fragmentation obligatory for tests // TF
    //fragmentationThreshold = msg->length()>>1;

    Mac80211Pkt *mac = encapsMsg(msg);
    EV << "packet " << msg << " received from higher layer, dest=" << mac->getDestAddr() << ", encapsulated\n";
    // frame based retries
    mac->setLongRetry(0);
    mac->setShortRetry(0);

    appendFrame(mac);

    // If the MAC is in the IDLE state, then start a new contention period
    if (state == IDLE && !endSifs->isScheduled()) {
        beginNewCycle();
    }
    else
    {
        EV << "enqueued, will be transmitted later, because I'm in state "<<stateName(state)<<" now"<<endl;
    }
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all needed
 * header fields.
 */
Mac80211Pkt *Mac80211a::encapsMsg(cMessage * netw)
{

    Mac80211Pkt *pkt = new Mac80211Pkt(netw->name());
    pkt->setLength(272);        // headerLength, including final CRC-field

    // copy dest address from the Control Info attached to the network
    // mesage by the network layer
    MacControlInfo* cInfo = static_cast<MacControlInfo*>(netw->removeControlInfo());

    EV <<"CInfo removed, mac addr="<< cInfo->getNextHopMac()<<endl;
    pkt->setDestAddr(cInfo->getNextHopMac());

    //delete the control info
    delete cInfo;

    //set the src address to own mac address (nic module id())
    pkt->setSrcAddr(myMacAddr);
    // set sequence number for fragmentation // TF
    EV<<"sequence number set to: "<<currentSequenceNumber()<<endl;
    pkt->setSequenceNumber(nextSequenceNumber());
    pkt->setMoreFrag(0);
  
    //encapsulate the network packet
    pkt->encapsulate(netw);
    EV <<"pkt encapsulated\n";
    
    return pkt;
}

cMessage *Mac80211a::decapsMsg(Mac80211Pkt *frame) {
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
void Mac80211a::handleLowerMsg(cMessage *msg)
{
    DestAddrList l;
    DestAddrList::iterator it;
    bool eFound = false;
    Mac80211Pkt *af = static_cast<Mac80211Pkt *>(msg);
        
    // end of the reception
    EV << "frame " << af << " received\n";
    addNeighbor(af);
    // special check, if I am in address list with unknown position
    if(af->kind() == ERTS)
    {
    	l = af->getDestAddrList();
    	for (it=l.begin();it!=l.end();it++)
	{
		if (*it == myMacAddr)
			eFound = true;
	}
	if (eFound)
	{
		EV<<"handling eRTS frame for me"<<endl;
		handleeRTSframe(af);
		suspendContention();
	}
	else
	{
		handleMsgNotForMe(af, af->getDuration());
	}
    }
    else if(af->getDestAddr() == myMacAddr) {
    	EV << "handling frame for me" << endl;
        handleMsgForMe(af);
    }
    else if(af->getDestAddr() == L2BROADCAST) {
    	EV << "handling broadcast frame" << endl;
        handleBroadcastMsg(af);
    }
    else {
    	EV << "handling frame not for me" << endl;
	if (af->getDuration()>0)
            handleMsgNotForMe(af, af->getDuration());
	else {
	    EV<<"Warning, duration negative: "<<af->getDuration()<<endl;
            handleMsgNotForMe(af, -af->getDuration());
	}
    }
}

void Mac80211a::handleLowerControl(cMessage *msg) 
{
    Mac80211Pkt* mf;
    switch(msg->kind()) {
    case COLLISION:
        mf = static_cast<Mac80211Pkt*>(msg->encapsulatedMsg()); 
    	EV << "PHY signaled collision to MAC" << endl;
	if ((mf->getDestAddr()==myMacAddr) || (eRtsForMe(mf))) {
	     if ((mf->kind()==DATA) || (mf->kind()==EDATA))
	     	RECORD_DATA_FRAME_ERROR(mf->length());
	     else
	     	RECORD_CONTROL_FRAME_ERROR(mf->length());
	}
	delete msg;
	break;
    case BITERROR:
        mf = static_cast<Mac80211Pkt*>(msg->encapsulatedMsg()); 
    	EV << "PHY signaled biterror to MAC" << endl;
        //handleMsgNotForMe(msg, EIFS); 
	// handle erts frames
	if ((mf->getDestAddr()==myMacAddr) || (eRtsForMe(mf))) {
	     if ((mf->kind()==DATA) || (mf->kind()==EDATA))
	     	RECORD_DATA_FRAME_ERROR(mf->length());
	     else
	     	RECORD_CONTROL_FRAME_ERROR(mf->length());
	}
	delete msg;
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
void Mac80211a::handleSelfMsg(cMessage * msg)
{
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
void Mac80211a::handleMsgNotForMe(cMessage *af, double duration)
{
    EV << "handleMsgNotForMe() called" << endl;

    // if the duration of the packet is null, then do nothing (to avoid
    // the unuseful scheduling of a self message)
    if ((duration != 0)&&(af->kind()!=ECTS)) {
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
        else if ((!noeCtsScheduled) || (!noDelayedAckScheduled)) {
	    EV<<"Not cancelling timers, in extended cycle."<<endl;
	}
	else {
            // if the MAC wait for another frame, it can delete its time out
            // (exchange is aborted)
	    EV<< "Cancelling timeout timer..."<<endl;
            if (timeout->isScheduled())
                cancelEvent(timeout);

            // is state == WFCTS or WFACK, the data transfer has failed ...

            // the node must defer for the time of the transmission
            EV << "NAV timer started, not QUIET: " << duration << endl;
            scheduleAt(simTime() + duration, nav);
            suspendContention();
            setState(QUIET);
        }
    }
    if ((state == CONTEND)&&(af->kind()!=ECTS)) {
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
void Mac80211a::handleMsgForMe(Mac80211Pkt *af)
{
    EV << "handle msg for me\n";

    switch (state)
    {
    case IDLE:     // waiting for the end of the contention period
    case CONTEND:  // or waiting for RTS

        // RTS or DATA accepted
        if (af->kind() == RTS) {
            suspendContention();
            handleRTSframe(af);
        }
	else if (af->kind() == ERTS) {
	    // this won't happen, because eRTS for me is identified earlier
	    suspendContention();
	    handleeRTSframe(af);
	}
        else if (af->kind() == DATA) {
            suspendContention();
            handleDATAframe(af);
        }
	else if (af->kind() == EDATA) {
	    suspendContention();
	    handleeDATAframe(af);
	}
        else
            EV << "in handleMsgForMe() IDLE/CONTEND, strange message (Typ: " <<frameTypeName(af->kind())<<"), darf das?"<<endl;
        break;

    case WFDATA:  // waiting for DATA
        if (af->kind() == DATA)
            handleDATAframe(af);
        else if (af->kind() == EDATA)
	    handleeDATAframe(af);
	else if (af->kind() == RTS) {
	    if (timeout->isScheduled())
	    	cancelEvent(timeout);
	    handleRTSframe(af);
	}
	else
            EV << "in handleMsgForMe() WFDATA, strange message (Typ: "<<frameTypeName(af->kind())<<"), darf das?\n";
        break;

    case WFACK:  // waiting for ACK

        if (af->kind() == ACK)
            handleACKframe(af);
        else
            EV << "in handleMsgForMe() WFACK, strange message (Typ: "<<frameTypeName(af->kind())<<"), darf das?";
        //delete af; // to early for fragmentation // TF
        break;

    case WFCTS:  // The MAC is waiting for CTS

        if (af->kind() == CTS)
            handleCTSframe(af);
	else if (af->kind() == ECTS)
	    handleeCTSframe(af);
        else
            EV << "in handleMsgForMe() WFCTS, strange message (Typ; "<<frameTypeName(af->kind())<<"), darf das?\n";
        break;


    case QUIET: // the node is currently deferring.

        // cannot handle any packet with its MAC adress
        delete af;
        break;

    case BUSY: // currently transmitting an ACK or a BROADCAST packet
        error("logic error: node is currently transmitting, can not receive "
              "(does the physical layer do its job correctly?)");
        break;

    default:
        error("unknown state %d", state);
    }
}

/**
 *  Handle aframe wich is expected to be an RTS. Called by
 *  HandleMsgForMe()
 */
void Mac80211a::handleRTSframe(Mac80211Pkt * af)
{
    // wait a short interframe space
    RECORD_CONTROL_FRAME_SUCCESS(af->length());
    endSifs->setContextPointer(af);
    noeCtsScheduled=true;
    noDelayedAckScheduled=true;
    scheduleAt(simTime() + SIFS, endSifs);
}

void Mac80211a::handleeRTSframe(Mac80211Pkt* af)
{
    EV<<"handleeRTSframe() called..."<<endl;
    RECORD_CONTROL_FRAME_SUCCESS(af->length());
    DestAddrList::iterator it;
    DestAddrList dal = af->getDestAddrList();
    endSifs->setContextPointer(af);
    currentControlDelay=0;

    cancelEvent(timeout);
    
    PhyControlInfo* pco = static_cast<PhyControlInfo*>(af->controlInfo());
    myOFDMStates = pco->getOFDMState();
    
    // search for my MAC address entry
    // relies on my MAC address being in this list (checked before in handleLowerMessage)
    it = dal.begin();
    while ((it!=dal.end()) && (*it!=myMacAddr))
    {
	currentControlDelay++;
	it++;
    }
    EV<<"currentControlDelay is "<<currentControlDelay<<endl;
    scheduleAt(simTime() + SIFS + currentControlDelay * (eCtsDuration + SIFS), endSifs);
    noeCtsScheduled=false;
}


/**
 *  Handle a frame which expected to be a DATA frame. Called by
 *  HandleMsgForMe()
 */
void Mac80211a::handleDATAframe(Mac80211Pkt * af)
{
    EV<<"handleDATAframe() was called"<<endl;
    RECORD_DATA_FRAME_SUCCESS(af->length());
    cancelEvent(timeout);  // cancel time-out event
    cMessage* toUpper;

    addToDefragBuffer(af);
    
    if (defragBufferComplete())
    	{
	    EV<<"all fragments received correctly."<<endl;
	    Mac80211Pkt* temp = defragBuffer.back();
	    toUpper = decapsMsg(temp);
	    // pass the packet to the upper layer
	    RECORD_PACKET_DELAY(simTime()-temp->creationTime());
	    RECORD_PACKET_NUMFRAG(af->getFragmentNumber()+1);
	    RECORD_PACKET_SUCCESS(toUpper->length());
	    sendUp(toUpper);
	    //clearDefragBuffer();
	}
    // wait a short interframe space
    endSifs->setContextPointer(af);
    scheduleAt(simTime() + SIFS, endSifs);
    noeCtsScheduled=true;
}

void Mac80211a::handleeDATAframe(Mac80211Pkt* af)
{
    EV<<"handleeDATAframe() was called"<<endl;
    RECORD_DATA_FRAME_SUCCESS(af->length());

    if (af->getControlDelay()==currentControlDelay)
    	EV<<"controlDelay unchanged ("<<currentControlDelay<<")"<<endl;
    else {
    	EV<<"controlDelay corrected from "<<currentControlDelay<<" to "<<af->getControlDelay()<<endl;
	currentControlDelay=af->getControlDelay();
    }

    cancelEvent(timeout);
    cMessage* toUpper;

    addToDefragBuffer(af);

    if (defragBufferComplete())
        {
	    Mac80211Pkt* temp = defragBuffer.back();
	    EV<<"all fragments received correctly."<<endl;
	    toUpper = decapsMsg(temp);
	    RECORD_PACKET_DELAY(simTime()-temp->creationTime());
	    RECORD_PACKET_NUMFRAG(af->getFragmentNumber()+1);
	    RECORD_PACKET_SUCCESS(toUpper->length());
	    sendUp(toUpper);
	}
     endSifs->setContextPointer(af);
     scheduleAt(simTime() + SIFS + currentControlDelay * (ackCtsDuration + SIFS), endSifs);
     noDelayedAckScheduled=false;
}


/**
 *  Handle a frame which is expected to be an ACK. Called by
 *  HandleMsgForMe(MACFrame* af)
 */
void Mac80211a::handleACKframe(Mac80211Pkt * af)
{
    int macAddr = af->getSrcAddr();
    bool removedDestListTail = false;
    EV<<"handleACKframe() called for "<<macAddr<<endl;
    RECORD_CONTROL_FRAME_SUCCESS(af->length());
    if (destList.size()>sendList.size())
    	error("sendList smaller than destList");
    if (destList.size()<sendList.size())
    	error("handleACKframe() [begin] destList smaller than sendList");
    // cancel time-out event
    cancelEvent(timeout);
    
    if (!extendedCycle) {
	    if (sendList[macAddr].bytesSent >= sendList[macAddr].frame->length()-272) {
		    RECORD_PACKET_RATE(sendList[macAddr].bytesSent/(simTime()-tempPacketRateTimeCounter));
		    // the transmission is acknowledged : initialize long_retry_counter
		    longRetryCounter = 0;
		    shortRetryCounter = 0;
	   
		    // removes the acknowledged packet from the queue

		    deleteFrame(sendList[macAddr].frame);
		    removeFromList(macAddr);
		    eraseFromDestList(macAddr);


		    delete af; // is this correct? seems like // TF

		    // if thre's a packet to send and if the channel is free then start a new contention period
		    if (sendList.empty()) {
		        setState(IDLE);
		    	beginNewCycle();
		    }
		    else {
		    	error("Strange: not in extended cycle, but more ACKs pending");
		    }
	    }
	    else {
		    sendList[macAddr].curFragNum++;
		    EV<<"Bits sent: "<<sendList[macAddr].bytesSent<<endl;
	      	    sendList[macAddr].bytesAcked=sendList[macAddr].bytesSent;
		    endSifs->setContextPointer(af);
		    scheduleAt(simTime() + SIFS, endSifs);
		    overheadTimeCounter+=SIFS;
		    noeCtsScheduled=true;
		    noDelayedAckScheduled=true;
	    }
    } 
    else { // extendedCycle == true
//       if (sendList[macAddr].bytesSent>sendList[macAddr].frame->length()-272)
//           error("uugh, i sent out too much data: %d",sendList[macAddr].bytesSent-sendList[macAddr].frame->length());
       if (sendList[macAddr].bytesSent>=sendList[macAddr].frame->length()-272) {// frame is completely transmitted
       	   EV<<"Frame sent out completely."<<endl;
	   tempPacketRateBitCounter+=sendList[macAddr].bytesSent;
	   if (destList.back()==macAddr)
	   	removedDestListTail=true;
	   else
	   	removedDestListTail=false;
           removeFromList(macAddr);
           deleteFrame(sendList[macAddr].frame);
	   eraseFromDestList(macAddr);
	   if (sendList.empty()) {
	   	RECORD_PACKET_RATE(tempPacketRateBitCounter/(simTime()-tempPayloadTimeCounter));
	   	delete af;
	   	setState(IDLE);
		EV<<"Everything sent out, nothing more in current sendList."<<endl;
	   	beginNewCycle();
	   } 
	   else {
	   	EV<<"sendList still holds frames for: "<<endl;
		SendList::iterator it;
		for (it=sendList.begin();it!=sendList.end();it++)
			EV<<it->first<<endl;
		EV<<"sendList size is "<<sendList.size()<<endl;
		if ((destList.back()!=macAddr) && (!removedDestListTail)) {
		    noDelayedAckScheduled=false;
		    scheduleAt(simTime()+SIFS+ackCtsDuration+delta,timeout);
		    EV<<"Waiting for another ACK..."<<endl;
		    delete af;
		}
		else {
	            noDelayedAckScheduled=true;
	            noeCtsScheduled=true;
		    endSifs->setContextPointer(af);
	            if (endSifs->isScheduled())
	                cancelEvent(endSifs);
		    scheduleAt(simTime()+SIFS,endSifs);
		    overheadTimeCounter+=SIFS;
		}
	   }
       }
       else {
           EV<<"Still fragments pending for "<<macAddr<<endl;
           sendList[macAddr].curFragNum++;
	   sendList[macAddr].bytesAcked=sendList[macAddr].bytesSent;
	   if (destList.back()!=macAddr) {
	   	// still ACKs pending
		noDelayedAckScheduled=false;
		scheduleAt(simTime()+SIFS+ackCtsDuration+delta,timeout);
		EV<<"Waiting for another ACK..."<<endl;
		delete af;
	   }
	   else {
	       noDelayedAckScheduled=true;
	       noeCtsScheduled=true;
	       endSifs->setContextPointer(af);
	       if (endSifs->isScheduled())
	           cancelEvent(endSifs);
	       scheduleAt(simTime()+SIFS,endSifs);
	   }
       }
    }
    if (destList.size()>sendList.size())
    	error("sendList smaller than destList");
    if (destList.size()<sendList.size())
    	error("handleACKframe() [end] destList smaller than sendList");
}


/**
 *  Handle a CTS frame. Called by HandleMsgForMe(Mac80211Pkt* af)
 */
void Mac80211a::handleCTSframe(Mac80211Pkt * af)
{
    // cancel time-out event
    cancelEvent(timeout);
    RECORD_CONTROL_FRAME_SUCCESS(af->length());
    
    // wait a short interframe space
    endSifs->setContextPointer(af);
    currentBitrate=BITRATES_80211a[af->getSuggestedMode()];
    EV<<"Bitrate set according to suggested mode "<<af->getSuggestedMode()<<endl;
    scheduleAt(simTime() + SIFS, endSifs);
    noeCtsScheduled=true;
    noDelayedAckScheduled=true;
}

void Mac80211a::handleeCTSframe(Mac80211Pkt* af)
{
    OFDMSnrListEntry ofdmState;
    Modelist modes;
    BasicAssignmentMessage* aMsg;

    EV<<"handleeCTSframe() called for "<<af->getSrcAddr()<<endl;
    RECORD_CONTROL_FRAME_SUCCESS(af->length());

    if (destList.size()>sendList.size())
    	error("sendList smaller than destList");
    if (destList.size()<sendList.size())
    	error("handleeCTSframe() [begin] destList smaller than sendList");

    cancelEvent(timeout);
    endSifs->setContextPointer(af);
    
    EV<<"Setting received marker for STA "<<af->getSrcAddr()<<endl;
    if (!sendList[af->getSrcAddr()].frame)
    	error("no frame in sendList");
    sendList[af->getSrcAddr()].eCts=1;

    // extract channel state information
    ofdmState = af->getOfdmState();
    
    // transform snr-values into modes
    for (int i=0;i<48;i++) {
    	EV<<"On subband "<<i<<" mode set to "<<getMode(ofdmState.snr[i])<<endl;
    	modes.mode[i]=getMode(ofdmState.snr[i]);
    }
    modes.simTime=simTime();
    EV<<"Modes extracted from eCTS frame."<<endl;

    // forward modearray to assignment algorithm
    aMsg = new BasicAssignmentMessage();
    aMsg->setMacAddr(af->getSrcAddr());
    aMsg->setModelist(modes);
    aMsg->setKind(ADD_TO_MODELIST);
    send(aMsg,toAssignment);
    EV<<"Modes array sent to assignment module."<<endl;

    if (destList.size()>sendList.size())
    	error("sendList smaller than destList");
    if (destList.size()<sendList.size())
    	error("handleeCTSframe() [end] destList smaller than sendList");
    if (af->getSrcAddr()==destList.back()) {
    	EV<<"Last eCTS received..."<<endl;
	requestAssignment();
        // set proper timeout // this is sufficient, if sender sets duration correct
        if (endSifs->isScheduled()) {
        	cancelEvent(endSifs);
		delete endSifs->contextPointer();
	}
	endSifs->setContextPointer(af);
        scheduleAt(simTime() + SIFS, endSifs);
    }
    else {
        delete af;
        scheduleAt(simTime()+SIFS+eCtsDuration+delta,timeout);
	EV<<"Waiting for another eCTS..."<<endl;
    }
}


/**
 *  Handle a broadcast packet. This packet is simply passed to the
 *  upper layer. No acknowledgement is needed.  Called by
 *  handleLowerMsg(Mac80211Pkt *af)
 */
void Mac80211a::handleBroadcastMsg(Mac80211Pkt *af)
{
    EV << "handle broadcast\n";
    if (state == BUSY)
        error("logic error: node is currently transmitting, can not receive "
              "(does the physical layer do its job correctly?)");

    sendUp(decapsMsg(af));
    delete af;
    if (state == CONTEND) beginNewCycle();
}


/**
 *  The node has won the contention, and is now allowed to send an
 *  RTS/DATA or Broadcast packet. The backoff value is deleted and
 *  will be newly computed in the next contention period
 */
void Mac80211a::handleEndContentionTimer()
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
	    if (extendedCycle) {
	    EV<<"More than one frame to send: Initiating extended cycle."<<endl;
	    	// send eRTS
	    	sendeRTSframe();
		setState(WFCTS);
	    }
            else if (rtsCts(sendList.begin()->second.frame)) {
                // send a RTS
                sendRTSframe();
                setState(WFCTS);
            }
            else { 
                sendDATAframe();
                setState(WFACK);
            }

            // broadcast packet
        }
        else {
            sendBROADCASTframe();

            // removes the packet from the queue without waiting for an acknowledgement
	    deleteFrame(sendList.begin()->second.frame);
        }
    }
    else {
        EV<<"endContentionTimer fired, state is: "<<stateName(state)<<endl;
        error("logic error: expiration of the contention timer outside of CONTEND/IDLE state, should not happen");
    }
}

/**
 *  Handle the NAV timer (end of a defering period). Called by
 *  HandleTimer(cMessage* msg)
 */
void Mac80211a::handleNavTimer()
{
    if (state != QUIET) {
        EV<<"navTimer fired in state "<<stateName(state)<<endl;
	//error("logic error: expiration of the NAV timer outside of the state QUIET, should not happen");
    }
    else
    // if there's a packet to send and if the channel is free, then start a new contention period
        beginNewCycle();
}


/**
 *  Handle the time out timer. Called by handleTimer(cMessage* msg)
 */
void Mac80211a::handleTimeoutTimer()
{
    DestAddrList::iterator it;
    EV<<"Timeout timer fired in state: "<<stateName(state)<<endl;
    if ((!extendedCycle) && ((state == WFCTS) || (state == WFACK))) {
	it=destList.begin();
	while ((it!=destList.end()) && (sendList[*it].eCts!=0))
		it++;
	// remove frame from current cycle
	sendList[*it].frame->setLongRetry(sendList[*it].frame->getLongRetry()+1);
	cancelFrame(*it);
        if (state != QUIET)
            beginNewCycle();
    }
    // if there's a packet to send and if the channel is free then
    // start a new contention period
    else {
    // handle missing eCTS and ACKs in multi OFDMA frame case
    if (state == WFCTS) {
    	EV<<"Missed CTS/eCTS..."<<endl;
    	// remove entry from sendList
	it=destList.begin();
	// search for first frame that is still missin an eCTS (that is the
	// frame we got a timeout for right now)
	while ((it!=destList.end()) && (sendList[*it].eCts!=0))
		it++;
	// remove frame from current cycle
	sendList[*it].frame->setLongRetry(sendList[*it].frame->getLongRetry()+1);
	cancelFrame(*it);
	
	if (!sendList.empty()) {
	if (sendList[destList.back()].eCts!=0) {
	    // all eCts received
	    Mac80211Pkt* m = new Mac80211Pkt("wlan-ects",ECTS);
	    m->setLength(LENGTH_ECTS);
	    endSifs->setContextPointer(m);
	    scheduleAt(simTime()+SIFS,endSifs);
	}
	else {
	    // at least one eCts pending
	    scheduleAt(simTime()+SIFS+eCtsDuration,timeout);
	}
	}
	else {
	    beginNewCycle();
	}
    }
    else if (state == WFACK) {
    	EV<<"Missed ACK..."<<endl;
    	// an ack was missed, update counters
	it=destList.begin();
	while ((it!=destList.end()) && (sendList[*it].bytesSent==sendList[*it].bytesAcked))
		it++;
	sendList[*it].bytesSent=sendList[*it].bytesAcked;
	sendList[*it].frame->setLongRetry(sendList[*it].frame->getLongRetry()+1);
	if (*it==destList.back()) {
	    testMaxAttempts(sendList[*it].frame);
	    EV<<"Last ACK missed, next round... ("<<*it<<", "<<destList.back()<<")"<<endl;
	    if (!sendList.empty())
	    	sendDATAframe();
	    else
	        beginNewCycle();
	}
	else {
	    testMaxAttempts(sendList[*it].frame);
	    EV<<"Have to wait for another ACK... ("<<*it<<", "<<destList.back()<<")"<<endl;
	    scheduleAt(simTime()+SIFS+ackCtsDuration+delta,timeout);
	}
    }
    }
}


/**
 *  Handle the end sifs timer. Then sends a CTS, a DATA, or an ACK
 *  frame
 */
void Mac80211a::handleEndSifsTimer()
{
    EV<<"handleEndSifsTimer()"<<endl;
    Mac80211Pkt *frame = static_cast<Mac80211Pkt *>(endSifs->contextPointer());
    radio->switchToSend();
    switch (frame->kind())
    {
    case RTS:
        sendCTSframe(frame);
	delete frame;
        break;
    case ERTS:
    	sendeCTSframe(frame);
	delete frame;
	break;
    case ECTS:
	EV<<"Sending DATA frames..."<<endl;
        sendDATAframe();
	delete frame;
    	break;
    case CTS:
        sendDATAframe();
	delete frame;
        break;
    case EDATA:
    case DATA:
        sendACKframe(frame);
        break;
    case ACK:
	EV<<"Sending DATA frames..."<<endl;
	sendDATAframe();
	delete frame;
	break;
    default:
        error("logic error: end sifs timer when previously received packet is not RTS/CTS/DATA/ACK, type was %d",frame->kind());
    }

    // don't need previous frame any more
    //delete frame;
}

/**
 *  Handle the end of transmission timer (end of the transmission
 *  of an ACK or a broadcast packet). Called by
 *  HandleTimer(cMessage* msg)
 */
void Mac80211a::handleEndTransmission()
{
    EV << "transmission of packet is over\n";
    if(state == BUSY) {
        if (defragBufferComplete()) { // wait for next fragment
            suspendContention();
            // if there's a packet to send and if the channel is free, then start a new contention period
            beginNewCycle();
	} else {
	    setState(IDLE); 
	}
    }
}

/**
 *  Send a DATA frame. Called by HandleEndSifsTimer() or
 *  handleEndContentionTimer()
 */
void Mac80211a::sendDATAframe()
{
    EV<<"sendDATAframe() was called..."<<endl;
    double minDuration;
    double temp;
    int newControlDelay=0;
    bool firstFrame=false;
    RECORD_DESTNUM(static_cast<int>(destList.size()));
    if (sendList.size()!=destList.size())
        error("sendList and destList differ in size: (%d, %d)",sendList.size(),destList.size());
    if (!extendedCycle) {
	    Mac80211Pkt *frame = static_cast<Mac80211Pkt *>(sendList[destList.back()].frame->dup());
	    currentBitrate = retrieveBitrate(frame->getDestAddr());
	    // build a copy of the frame in front of the queue'
	    PhyControlInfo *pco = new PhyControlInfo(currentBitrate);

	    pco->setMagicNumber(0);
	    
	    frame->setControlInfo(pco);
	    frame->setSrcAddr(myMacAddr);
	    frame->setKind(DATA); 

	    radio->setBitrate(currentBitrate);

	    if (sendList[destList.back()].maxFragNum > 0)
	    {
		if ((sendList[destList.back()].frame->length()-272-sendList[destList.back()].bytesSent)>fragmentationThreshold) {
		    frame->setFragmentLength(272+fragmentationThreshold);
		    frame->setMoreFrag(1);
		    sendList[destList.back()].bytesSent+=frame->length()-272;
		}
		else {
		    frame->setFragmentLength(sendList[destList.back()].frame->length()-fragmentationThreshold*(sendList[destList.back()].maxFragNum)); // last fragment might be less than fragmentationThreshold // TF
		    frame->setMoreFrag(0);
		    sendList[destList.back()].bytesSent+=frame->length()-272;
	       }
	   } 
	   else
	   {
		frame->setMoreFrag(0);
		sendList[destList.back()].bytesSent+=frame->length()-272;
	   }

	    if (frame->getDestAddr() != L2BROADCAST) {
		// duration is set to number of pending fragments times (SIFS+ACK+SIFS) - SIFS (last SIFS not needed)
		frame->setDuration((sendList[destList.back()].maxFragNum-sendList[destList.back()].curFragNum+1)*(2*SIFS + ackCtsDuration)-SIFS);
		frame->setKind(DATA);
	    }
	    else {
	        EV<<"sending broadcast frame out of sendDATAfram()"<<endl;
		frame->setDuration(0); // duration for broadcast frames
	    }
	    EV<<"sending fragment "<<sendList[destList.back()].curFragNum<<" of "<<sendList[destList.back()].maxFragNum<<endl;


	    // schedule time out // this is much too high, crap 
	    scheduleAt(simTime() + 3*SIFS + packetDuration(frame->length(),currentBitrate) + ackCtsDuration + delta, timeout);
	    EV << "sending DATA  to " << frame->getDestAddr() << " with bitrate " << currentBitrate << endl;
	    // send DATA frame
	    frame->setAirTime(packetDuration(frame->length(),currentBitrate));
	    frame->setOFDMABitRate(currentBitrate);
	    RECORD_SELECTED_MODE(getModeFromBitrate(static_cast<long>(currentBitrate)),48);
	    RECORD_DATA_FRAME_RATE(frame->getOFDMABitRate());
	    RECORD_TOTAL_FRAME_RATE(frame->getOFDMABitRate());
	    RECORD_FRAGMENTLENGTH(frame->length());
	    if (tempOverheadTimeCounter>0.0) {
	    	overheadTimeCounter=simTime()-tempOverheadTimeCounter;
		tempOverheadTimeCounter=-1.0;
	    }
	    overheadTimeCounter+=SIFS;
	    overheadTimeCounter+=ackCtsDuration;
	    overheadTimeCounter+=packetDuration(272,currentBitrate);
	    payloadTimeCounter+=packetDuration(frame->length(),currentBitrate)-SYMBOL_TIME;
	    payloadBitCounter+=frame->length()-272;
	    overheadBitCounter+=272;
	    overheadBitCounter+=LENGTH_ACK;
	    sendDown(frame);
    }
    else {
	// determine minimum fragment length
	minDuration = packetDuration(fragmentationThreshold,controlBitrate); // let fragthresh define longest duration
	temp = 0;
	DestAddrList::iterator dit;
	EV<<"sendList size is "<<sendList.size()<<endl;
	EV<<"destList size is "<<destList.size()<<endl;
	for (dit=destList.begin();dit!=destList.end();dit++) {
	    EV<<"Bits sent (without header): "<<sendList[*dit].bytesAcked;
		if (sendList[*dit].frame) {
			EV<<"Bits left: "<<sendList[*dit].frame->length()-272-sendList[*dit].bytesAcked<<endl;
		}
		else
			EV<<"Frame pointer unset..."<<endl;
		EV<<"Calculating minimum fragment duration."<<endl;
		temp = packetDuration(sendList[*dit].frame->length()-sendList[*dit].bytesAcked,getOFDMABitRate(sendList[*dit].frame->getDestAddr()),sendList.size());
		if (temp < minDuration)
			minDuration = temp;
	}
	if (timeout->isScheduled())
		cancelEvent(timeout);
	if ((SIFS+minDuration+sendList.size()*(SIFS+ackCtsDuration)+delta)<0) {
	     EV<<"Warning timeout negative: "<<SIFS+minDuration+sendList.size()*(SIFS+ackCtsDuration)+delta<<endl;
	     EV<<"Frame length: "<<sendList[*dit].frame->length()<<", bits sent: "<<sendList[*dit].bytesSent<<", bits acked: "<<sendList[*dit].bytesAcked<<endl;
	}
	scheduleAt(simTime()+SIFS+minDuration+sendList.size()*(SIFS+ackCtsDuration)+delta,timeout);
	firstFrame=true;
	long tempTotalBitrate=0;
	for (dit=destList.begin();dit!=destList.end();dit++) {
	// set appropiate length to each frame
	// set assignment
	// send down each frame
	    Mac80211Pkt *frame = static_cast<Mac80211Pkt *>(sendList[*dit].frame->dup());
	    PhyControlInfo *pco = new PhyControlInfo(getOFDMABitRate(frame->getDestAddr()));
	    pco->setAssignment(myAssignment);
	    pco->setMagicNumber(currentMagicNumber);
	    EV<<"Length before: "<<frame->length()-sendList[*dit].bytesAcked<<", and after: ";
	    // length calculation with respect to ePLCP header length
	    frame->setFragmentLength(static_cast<int>(ceil(getOFDMABitRate(frame->getDestAddr())*frameDuration(minDuration,sendList.size())-0.00001)));
	    EV<<frame->length()<<endl;
	    EV<<"Non-integer calculation: "<<getOFDMABitRate(frame->getDestAddr())*frameDuration(minDuration,sendList.size())<<endl;
	    sendList[*dit].bytesSent+=frame->length()-272;
	    frame->setAirTime(minDuration);
	    frame->setDuration(minDuration+SIFS+ackCtsDuration+DIFS);
	    frame->setKind(EDATA);
	    frame->setControlInfo(pco);
	    if (sendList[*dit].bytesSent>=(sendList[*dit].frame->length()-272))
	    	frame->setMoreFrag(0);
	    else
	    	frame->setMoreFrag(1);
	    frame->setFragmentNumber(sendList[*dit].curFragNum);
	    frame->setControlDelay(newControlDelay++);
	    EV<<"OFDMA-Bitrate for frame is: "<<getOFDMABitRate(*dit)<<endl;
	    frame->setOFDMABitRate(getOFDMABitRate(*dit));
	    EV<<"Sending down frame for: "<<frame->getDestAddr()<<endl;
	    recordModesToSink(*dit); // RECORD_SELECTED_MODE
	    RECORD_DATA_FRAME_RATE(frame->getOFDMABitRate());
	    tempTotalBitrate+=frame->getOFDMABitRate();
	    RECORD_FRAGMENTLENGTH(frame->length());
	    if ((tempOverheadTimeCounter>0.0)&&(firstFrame)) {
	        overheadTimeCounter=simTime()-tempOverheadTimeCounter;
	        tempOverheadTimeCounter=-1.0;
	    }
	    overheadTimeCounter+=SIFS;
	    overheadTimeCounter+=ackCtsDuration;
	    overheadTimeCounter+=packetDuration(272,frame->getOFDMABitRate(),sendList.size());
	    payloadTimeCounter+=packetDuration(frame->length(),frame->getOFDMABitRate(),sendList.size())-packetDuration(272,frame->getOFDMABitRate(),sendList.size());
	    payloadBitCounter+=frame->length()-272;
	    overheadBitCounter+=272;
	    overheadBitCounter+=LENGTH_ACK;
	    sendDown(frame);
        }
	RECORD_TOTAL_FRAME_RATE(tempTotalBitrate);
    }
    // update state and display
    setState(WFACK);
}


/**
 *  Send an ACK frame.Called by HandleEndSifsTimer()
 */
void Mac80211a::sendACKframe(Mac80211Pkt * af)
{
    // send ACK frame
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-ack");

    PhyControlInfo *pco = new PhyControlInfo(controlBitrate);
    frame->setControlInfo(pco);
    
    frame->setKind(ACK);
    frame->setLength(LENGTH_ACK);

    // the dest address must be the src adress of the RTS or the DATA
    // packet received. The src adress is the adress of the node
    frame->setSrcAddr(myMacAddr);
    frame->setDestAddr(af->getSrcAddr());
    frame->setDuration(0.0);

    radio->setBitrate(controlBitrate);
   
    frame->setAirTime(packetDuration(frame->length(),controlBitrate));
    sendDown(frame);
    EV << "sent ACK frame!\n";

    // update state and display
    setState(BUSY);
}


/**
 *  Send a RTS frame.Called by handleContentionTimer()
 */
void Mac80211a::sendRTSframe()
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-rts");
    currentBitrate = retrieveBitrate(sendList.begin()->second.frame->getDestAddr());
    
    PhyControlInfo *pco = new PhyControlInfo(controlBitrate);
    pco->setMagicNumber(0);
    
    frame->setControlInfo(pco);

    frame->setKind(RTS);
    frame->setLength(LENGTH_RTS);
    overheadBitCounter+=LENGTH_RTS;

    // the src adress and dest address are copied in the frame in the queue (frame to be sent)
    frame->setSrcAddr(sendList.begin()->second.frame->getSrcAddr());
    frame->setDestAddr(sendList.begin()->second.frame->getDestAddr());
    frame->setDuration(3 * SIFS + packetDuration(LENGTH_CTS,controlBitrate) +	// TODO: this is ok, NAV is corrected later in fragment burst transmission, fix it? // TF
                       packetDuration(sendList.begin()->second.frame->length(), currentBitrate) +
                       packetDuration(LENGTH_ACK,controlBitrate));

    radio->setBitrate(controlBitrate);
    
    // schedule time-out
    //scheduleAt(simTime() + 3*SIFS + 2*ackCtsDuration + packetDuration(fromUpperLayer.front()->length(),currentBitrate) + delta, timeout);
    //
    // mmmmh??? // TF
    scheduleAt(simTime() + packetDuration(LENGTH_RTS,controlBitrate) + 2*SIFS + ackCtsDuration + delta, timeout);

    // send RTS frame
    frame->setAirTime(packetDuration(frame->length(),controlBitrate));
    sendDown(frame);

    // update state and display
    setState(WFCTS);
}

void Mac80211a::sendeRTSframe() 
{
    Mac80211Pkt* frame = new Mac80211Pkt("wlan-erts");
    SendList::iterator its;
   
/*    EV<<"Generating sendList:"<<endl;
    // generate destination addresslist, from frame list
    for (its=sendList.begin();its!=sendList.end();its++) {
    	destList.push_back(its->first);
	EV<<"Added "<<its->first<<" to sendList..."<<endl;
    }  */
    EV<<"sendeRTSframe() called"<<endl;
    EV<<"sendList size is "<<sendList.size()<<endl;
    EV<<"destList size is "<<destList.size()<<endl;

    currentBitrate = controlBitrate; // assume lowest possible bitrate

    PhyControlInfo* pco = new PhyControlInfo(controlBitrate);
    pco->setMagicNumber(0);

   
    frame->setControlInfo(pco);
    frame->setKind(ERTS);
    frame->setSrcAddr(myMacAddr);

    frame->setDestAddrList(destList);
    
    frame->setLength(LENGTH_RTS+(destList.size()-1)*48);
    overheadBitCounter+=frame->length();
    // we need frame length in addition to this
    frame->setDuration(SIFS+destList.size()*(SIFS+eCtsDuration)+destList.size()*(SIFS+ackCtsDuration)+packetDuration(fragmentationThreshold,controlBitrate)+SIFS);
    frame->setAirTime(packetDuration(frame->length(),controlBitrate));

    radio->setBitrate(controlBitrate);

    scheduleAt(simTime() + packetDuration(frame->length(),controlBitrate) + 2*SIFS + eCtsDuration + delta, timeout);
    sendDown(frame);
    setState(WFCTS);
}

/**
 *  Send a CTS frame.Called by HandleEndSifsTimer()
 */
void Mac80211a::sendCTSframe(Mac80211Pkt * af)
{
    Mac80211Pkt *frame = new Mac80211Pkt("wlan-cts");
    
    PhyControlInfo *pco = new PhyControlInfo(controlBitrate);
    pco->setMagicNumber(0);
    frame->setControlInfo(pco);

    frame->setKind(CTS);
    frame->setLength(LENGTH_CTS);
    overheadBitCounter+=LENGTH_CTS;

    // the dest adress must be the src adress of the RTS received. The
    // src adress is the adress of the node
    frame->setSrcAddr(myMacAddr);
    frame->setDestAddr(af->getSrcAddr());
    frame->setDuration(af->getDuration() - SIFS - ackCtsDuration);
    frame->setSuggestedMode(getModeFromBitrate(static_cast<int>(retrieveBitrate(frame->getDestAddr()))));

    frame->setAirTime(ackCtsDuration);

    radio->setBitrate(controlBitrate);
    
    //scheduleAt(simTime() + af->getDuration() - packetDuration(LENGTH_RTS, BITRATES_80211a[0]) - 2 * SIFS + delta, timeout);
    scheduleAt(simTime() + af->getDuration() - ackCtsDuration - SIFS + delta, timeout);

    // send CTS frame
    sendDown(frame);

    // update state and display
    setState(WFDATA);
}

void Mac80211a::sendeCTSframe(Mac80211Pkt* af)
{
    Mac80211Pkt* frame = new Mac80211Pkt("wlan-ects");

    PhyControlInfo* pco = new PhyControlInfo(controlBitrate);
    pco->setMagicNumber(0);
    frame->setControlInfo(pco);

    frame->setKind(ECTS);
    frame->setLength(LENGTH_ECTS);
    overheadBitCounter+=LENGTH_ECTS;

    frame->setSrcAddr(myMacAddr);
    frame->setDestAddr(af->getSrcAddr());

    frame->setDuration(af->getDuration()-SIFS-eCtsDuration);

    EV<<"Attaching OFDM state information to eCTS."<<endl;
    frame->setOfdmState(myOFDMStates);

    radio->setBitrate(controlBitrate);

    // TODO state clear why this is the desired timeout
    scheduleAt(simTime() + af->getDuration()+delta,timeout);

    frame->setAirTime(packetDuration(frame->length(),controlBitrate));

    EV<<"Sending eCTS frame."<<endl;
    sendDown(frame);

    setState(WFDATA);
}

/**
 *  Send a BROADCAST frame.Called by handleContentionTimer()
 */
void Mac80211a::sendBROADCASTframe()
{
    // send a copy of the frame in front of the queue
    Mac80211Pkt *frame = static_cast<Mac80211Pkt *>(sendList.begin()->second.frame->dup());
    double br = retrieveBitrate(frame->getDestAddr());
    PhyControlInfo *pco = new PhyControlInfo(br);
    pco->setMagicNumber(0);
    frame->setControlInfo(pco);
    frame->setKind(BROADCAST);
    frame->setAirTime(packetDuration(frame->length(),br));
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
void Mac80211a::beginNewCycle()
{
    MultiMacPktList::iterator it;
    tempPacketRateTimeCounter=simTime();
    tempPacketRateBitCounter=0;
    int test=0;

    if ((overheadTimeCounter>0)&&(payloadTimeCounter>0)) {
    	RECORD_TIMEOVERHEAD_RATE(overheadTimeCounter/(overheadTimeCounter+payloadTimeCounter));
	overheadTimeCounter=-1.0;
	payloadTimeCounter=-1.0;
    }
    if ((overheadBitCounter>0)&&(payloadBitCounter>0)) {
    	RECORD_BITOVERHEAD_RATE(static_cast<double>(overheadBitCounter)/(static_cast<double>(overheadBitCounter)+static_cast<double>(payloadBitCounter)));
	overheadBitCounter=0;
	payloadBitCounter=0;
    }
    tempOverheadTimeCounter=simTime();

    // before trying to send one more time a packet, test if the
    // maximum retry limit is reached. If it is the case, then
    // delete the packet and send the next packet.
    EV<<"beginNewCycle() was called"<<endl;
    extendedCycle=false;
    //testMaxAttempts();
    EV<<"fromUpperLayerByDest.size() = "<<fromUpperLayerByDest.size()<<endl;
    if (!fromUpperLayerByDest.empty()) {
	it=fromUpperLayerByDest.begin();
        while (test<fromUpperLayerByDest.size()) {
	    if (!it->second.empty()) {
                testMaxAttempts(it->second.front());
		}
	    it++;
	    test++;
	}
    }
    sendList.clear();
    destList.clear();
    EV<<"Cleared sendList..."<<endl;
    clearAssignmentList();

    if (!fromUpperLayer.empty()) {

        // look if the next packet is unicast or broadcast
        nextIsBroadcast = (fromUpperLayer.front()->getDestAddr() == L2BROADCAST);

	if (!nextIsBroadcast) {	// fragment non-broadcast frames if necessary
		if (useOFDMAextensions)
		{
			// set magic number, fairly uniqe because macAddr is uniqe
			currentMagicNumber=fromUpperLayer.front()->getSequenceNumber()*100+myMacAddr;
			extendedCycle=true;
			// establish sendlist
    			// select frames to send out, relies on fact that fromUpperLayerByDest.size()<=24 
    			MultiMacPktList::iterator it;
			EV<<"sendList.size() is "<<sendList.size()<<endl;
			EV<<"destList.size() is "<<destList.size()<<endl;
			EV<<"fromUpperLayerByDest.size() is "<<fromUpperLayerByDest.size()<<endl;
    			for (it=fromUpperLayerByDest.begin();it!=fromUpperLayerByDest.end();it++)
    				{
					EV<<"beginNewCycle(): Adding frame to sendList for dest: "<<it->first<<endl;
    					if ((!it->second.empty())&&(it->first!=L2BROADCAST)) {
						sendList[it->first].frame=it->second.front();
						sendList[it->first].eCts=0;
						sendList[it->first].bytesAcked=0;
						sendList[it->first].bytesSent=0;
						sendList[it->first].curFragNum=0;
						sendList[it->first].maxFragNum=0;
						destList.push_back(it->first);
					} 
					else {
					    error("empty frame pointer in %d",it->first);
					}
    				}
			EV<<"sendList.size() is "<<sendList.size()<<endl;
			EV<<"destList.size() is "<<destList.size()<<endl;
		} // end (useOFDMAextensions)
		else
		{
		if (fromUpperLayer.front()->length()-272 > fragmentationThreshold) {
		    sendList[fromUpperLayer.front()->getDestAddr()].maxFragNum = (fromUpperLayer.front()->length()-272)/fragmentationThreshold;
		    if ((fromUpperLayer.front()->length()-272)%fragmentationThreshold == 0)
			sendList[fromUpperLayer.front()->getDestAddr()].maxFragNum--;
	   	    sendList[fromUpperLayer.front()->getDestAddr()].curFragNum = 0;
		    sendList[fromUpperLayer.front()->getDestAddr()].bytesSent = 0;
		    sendList[fromUpperLayer.front()->getDestAddr()].bytesAcked = 0;
		}
		else {
		    sendList[fromUpperLayer.front()->getDestAddr()].bytesSent = 0;
		    sendList[fromUpperLayer.front()->getDestAddr()].bytesAcked = 0;
		    sendList[fromUpperLayer.front()->getDestAddr()].maxFragNum = 0;
	            sendList[fromUpperLayer.front()->getDestAddr()].curFragNum = 0;
		}
		EV<<"Adding single frame to sendList..."<<endl;
		sendList[fromUpperLayer.front()->getDestAddr()].frame = fromUpperLayer.front();
		sendList[fromUpperLayer.front()->getDestAddr()].bytesAcked = 0;
		sendList[fromUpperLayer.front()->getDestAddr()].bytesSent = 0;
		destList.push_back(fromUpperLayer.front()->getDestAddr());
		}
	}
	else
	{	
		EV<<"Adding single broadcast frame to sendList..."<<endl;
		sendList[fromUpperLayer.front()->getDestAddr()].frame = fromUpperLayer.front();
		sendList[fromUpperLayer.front()->getDestAddr()].bytesAcked = 0;
		sendList[fromUpperLayer.front()->getDestAddr()].bytesSent = 0;
	}

        // if the channel is free then wait a random time and transmit
        setState(CONTEND);
        if(!contention->isScheduled()) {
            if(rssi < busyRSSI) {
                if(remainingBackoff < DIFS) remainingBackoff = DIFS;
                scheduleAt(simTime() + remainingBackoff, contention);
            }
            else {
                incrementShortRetry();
            }
        }
    }
    else {
        if(!contention->isScheduled()) {
            scheduleAt(simTime() + remainingBackoff, contention);
        }
        setState(IDLE);
    }
}

/**
 * Compute the backoff value.
 */
double Mac80211a::backoff() {
    return ((double) intrand(contentionWindow() + 1)) * ST;
}

/**
 *  Compute the contention window with the binary backoff
 *  algorithm.
 */
int Mac80211a::contentionWindow()
{
    int cw = CW_MIN << longRetryCounter;
    if(cw > CW_MAX) cw = CW_MAX;
    return cw;
}

/**
 *  Test if the maximal retry limit is reached, and delete the
 *  frame to send in this case.
 */
void Mac80211a::testMaxAttempts()
{
    if (longRetryCounter > LONG_RETRY_LIMIT) {
        EV << "long retry limit reached " << endl;
        // initialize counter
        longRetryCounter = 0;
        shortRetryCounter = 0;
        // delete the frame to transmit
        Mac80211Pkt *temp = fromUpperLayer.front();
        fromUpperLayer.pop_front();
        temp->setName("MAC ERROR");
        temp->setKind(PACKET_DROPPED);
	RECORD_PACKET_ERROR(temp->length());
        sendControlUp(temp);
    }
}

// for frame based retry counting
void Mac80211a::testMaxAttempts(Mac80211Pkt* frame) {
    EV<<"TestMaxAttempts() was called"<<endl;
    EV<<"Testing max attempts for "<<frame->getDestAddr()<<endl;
    if (frame->getLongRetry() > LONG_RETRY_LIMIT) {
        EV << "long retry limit reached " << endl;
        // delete the frame to transmit
        Mac80211Pkt *temp = static_cast<Mac80211Pkt*>(frame->dup());
        temp->setName("MAC ERROR");
        temp->setKind(PACKET_DROPPED);
	RECORD_PACKET_ERROR(temp->length());
        sendControlUp(temp);
	eraseFromDestList(frame->getDestAddr());
	deleteFrame(frame);
    }
    EV<<"TestMaxAttempts() has finished"<<endl;
}

/**
 * Handle change nofitications. In this layer it is usually
 * information about the radio channel, i.e. if it is IDLE etc.
 */
void Mac80211a::receiveBBItem(int category, const BBItem *details, int scopeModuleId)
{
    Enter_Method_Silent();
    BasicLayer::receiveBBItem(category, details, scopeModuleId);
    if(category == catRadioState) {
        radioState = static_cast<const RadioState *>(details)->getState();
    }
    else if(category == catRSSI) {
        // to do: restart state machine on this value
        rssi = static_cast<const RSSI *>(details)->getRSSI();
        // beginning of a reception
	EV<<"New RSSI value received from PHY "<<rssi<<endl;
        if(rssi > busyRSSI)
        {   
            EV << "medium busy" << endl;
            suspendContention();

            // if there's a SIFS period
            if (endSifs->isScheduled() && (noeCtsScheduled) && (noDelayedAckScheduled))
            {
                // delete the previously received frame
                delete static_cast<Mac80211Pkt *>(endSifs->contextPointer());

                // cancel the next transmission
                cancelEvent(endSifs);

                // state in now IDLE or CONTEND
                if ((fromUpperLayer.empty())) // && (curFragmentNumber == maxFragmentNumber)) // ??? // TF
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
 *  extended to support fragmentation
 */
double Mac80211a::timeOut(frameType_802_11 type, double br)
{
    double time_out = 0;
    
    switch (type)
    {
    case RTS:
        time_out = SIFS + packetDuration(LENGTH_RTS, br) + packetDuration(LENGTH_CTS, br) + delta;
        break;
    case DATA:
        time_out = SIFS + packetDuration(fromUpperLayer.front()->length(), br) + packetDuration(LENGTH_ACK, br) + delta;
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
double Mac80211a::packetDuration(int bits, double br)
{
    return bits / br + SYMBOL_TIME;
}

double Mac80211a::packetDuration(int bits, double br, int numSta) {
    // TODO fix duration calc
    return bits / br + (1+ceil((6+48*(3+ceil(log2(numSta)))/18))) * SYMBOL_TIME;
}

/** @brief calculation duration of 802.11a MAC frame whithout any headery from given duration, whith header */
double Mac80211a::frameDuration(double dur, int numSta) {
    //remove ePLCP header duration
    return dur - (1+ceil((6+48*(3+ceil(log2(numSta)))/18))) * SYMBOL_TIME;
}

/** @brief calculation duration of 802.11a MAC frame whithout any headery from given duration, whith header */
double Mac80211a::frameDuration(double dur) {
    //remove PLCP header duration
    return dur - SYMBOL_TIME;
}

const char *Mac80211a::stateName(State state)
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

const char *Mac80211a::frameTypeName(int kind) 
{
#define CASE(x) case x: s=#x; break
    const char *s ="UNKNOWN";
    switch(kind)
    {
        CASE(RTS);
	CASE(CTS);
	CASE(DATA);
	CASE(BROADCAST);
	CASE(ACK);
	CASE(ECTS);
	CASE(EDATA);
    }
    return s;
}

void Mac80211a::setState(State newState)
{
    if (state==newState)
        EV << "staying in state " << stateName(state) << "\n";
    else
        EV << "state " << stateName(state) << " --> " << stateName(newState) << "\n";
    state = newState;
}

void Mac80211a::suspendContention()  {
    // if there's a contention period
    if(contention->isScheduled()) {
        // update the backoff window in order to give higher priority in
        // the next battle
        remainingBackoff = contention->arrivalTime() - simTime();
        EV << "suspended backoff timer, remaining backoff time: " << remainingBackoff << endl;
        cancelEvent(contention);
    }
    else if(remainingBackoff == 0) {
        remainingBackoff = backoff();
        EV << " some event happend that needs a new backoff: "  << remainingBackoff << endl;
    }
}

long Mac80211a::retrieveBitrate(int destAddress) {
    long bitrate = defaultBitrate;
    NeighborList::iterator it;
    if(autoBitrate && (destAddress != L2BROADCAST)) {
        it = findNeighbor(destAddress);
        if((it != neighbors.end()) && (it->age > (simTime() - neighborhoodCacheMaxAge))) {
            bitrate = it->bitrate;
        }
    }
    return bitrate;
}

void Mac80211a::incrementShortRetry() {
    SendList::iterator it;
    for (it=sendList.begin();it!=sendList.end();it++) {
        it->second.frame->setShortRetry(it->second.frame->getShortRetry()+1);
        if(it->second.frame->getShortRetry() >= SHORT_RETRY_LIMIT) {
           EV << "short retry limit reached for "<<it->second.frame->getDestAddr() << endl;
	   it->second.frame->setShortRetry(0);
           it->second.frame->setLongRetry(it->second.frame->getLongRetry()+1);
        }
    }
}

void Mac80211a::addNeighbor(Mac80211Pkt *af) {
    int srcAddress = af->getSrcAddr();
    NeighborList::iterator it = findNeighbor(srcAddress);
    double snr = static_cast<PhyControlInfo *>(af->controlInfo())->getSnr();
    long bitrate = BITRATES_80211a[getMode(snr)];
    NeighborEntry entry;
    

    if(it != neighbors.end()) {
        it->age = simTime();
        it->bitrate = bitrate;
    }
    else {
        if(neighbors.size() < neighborhoodCacheSize) {
            entry.address = srcAddress;
            entry.age = simTime();
            entry.bitrate = bitrate;
            neighbors.push_back(entry);
        }
        else {
            it = findOldestNeighbor();
            if(it != neighbors.end()) {
                it->age = simTime();
                it->bitrate = bitrate;
                it->address = srcAddress;
            }
        }
    }
    EV << "updated information for neighbor: " << srcAddress
       << " snr: " << snr << " bitrate: " << bitrate << endl;
}

void Mac80211a::finish() {
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
    clearDefragBuffer();
    delete bufState;
}


/************************************************************************************
 * Author: Thomas Freitag <tf@uni-paderborn.de>
 * 
 * additional functionality for fragmentation 
 ************************************************************************************/


/**
 * returns true if frame is last fragment or no fragmentation is used
 */
bool Mac80211a::isLastFrag(Mac80211Pkt* af) {
    return !af->getMoreFrag();
}

/**
 * checks and sets fragment number for given frame (see standard clause 7.1.3.4)
 * this should be realized in Mac80211Pkt::setFragmentNumber(int)
 */
void Mac80211a::setFragmentNumber(Mac80211Pkt* af, int num) {
    if ((num>=0) && (num <=15))
	    af->setFragmentNumber(num);
    else    
            error("Fragment number %d not in range (0-15)",num);
}

/** 
 * @brief maintaines defragmentation queue (queue of fragments received)
 * frame is added to buffer iff:
 * 	(1) buffer is empty, or
 * 	(2) last frame in buffer has same sequence number and preceeding fragment number 
 * frame is ignored if a frame with equal fragment number already exists in buffer
 */
void Mac80211a::addToDefragBuffer(Mac80211Pkt* af) {
   if (defragBuffer.empty()) {
   	if (af->getMoreFrag())
		defragBufferIsComplete = false;
	else
		defragBufferIsComplete = true;
   	defragBuffer.push_back(af);	// add fragment if buffer empty
   } else {
   	Mac80211Pkt* temp = defragBuffer.back();
	defragBuffer.pop_back();
   	if (af->getMoreFrag())
		defragBufferIsComplete = false;
	else
		defragBufferIsComplete = true;
	if (temp->getSequenceNumber() != af->getSequenceNumber())
		clearDefragBuffer();		// clear buffer if fragment with new sequence number arrives
	defragBuffer.push_back(temp);
   	defragBuffer.push_back(af);	// add consecutive fragment
	}
}


/** 
 * @brief checks if complete frame is held in defrag buffer
 */
bool Mac80211a::defragBufferComplete() {
/*   Mac80211Pkt* temp;
   ev<<"defragBufferComplete(): called by "<<myMacAddr<<endl; 
   temp = defragBuffer.back();
   ev<<"defragBufferComplete(): frame extracted "<<endl; 
   ev<<"defragBufferComplete(): buffer.empty="<<defragBuffer.empty()<<endl;
   if (temp)
   	return temp->getMoreFrag();
   else
   	return false;*/
   return defragBufferIsComplete;
}

void Mac80211a::clearDefragBuffer() {
    MacPktList::iterator it;
    for(it = defragBuffer.begin(); it != defragBuffer.end(); ++it) {
       delete (*it);
    }
    defragBuffer.clear();
}

/**
 * @brief provides sequence numbers from 0 to 2^12-1 (see standard clause 7.1.3.4)
 */
int Mac80211a::nextSequenceNumber() {
	if (sequenceNumber < 4095)
		return sequenceNumber++;
	else
	{
		sequenceNumber=0;
		return sequenceNumber;
	}
}

int Mac80211a::currentSequenceNumber() {
	return sequenceNumber;
}

/** @brief deletes frame from standard link buffer and destination address map */
void Mac80211a::deleteFrame(Mac80211Pkt* f) {
	MacPktList::iterator it;
	int macAddr = f->getDestAddr();

	EV<<"deleteFrame() was called for addr "<<f->getDestAddr()<<endl;
	
	// delete pointer from standard MAC queue
	it=fromUpperLayer.begin();
	//EV<<"fromUpperLayer() size before "<<fromUpperLayer.size()<<endl;
	while ((it!=fromUpperLayer.end())&&(*it!=f))
		it++;
	if (*it==f)
		fromUpperLayer.erase(it);
	else
	    EV<<"Tried to remove non existing frame from fromUpperLayer"<<endl;
	//EV<<"fromUpperLayer() size after "<<fromUpperLayer.size()<<endl;

	// delete pointer from MAC map
	//EV<<"fromUpperLayerByDest() size before "<<fromUpperLayerByDest[macAddr].size()<<endl;
	it=fromUpperLayerByDest[macAddr].begin();
	while ((it!=fromUpperLayerByDest[macAddr].end())&&(*it!=f))
		it++;
	if (*it==f)
		fromUpperLayerByDest[macAddr].erase(it);
	else
	    EV<<"Tried to remove non existing frame from fromUpperLayerByDest"<<endl;
	//EV<<"fromUpperLayerByDest() size after "<<fromUpperLayerByDest[macAddr].size()<<endl;
	
	if (fromUpperLayerByDest[macAddr].empty()) {
		fromUpperLayerByDest.erase(macAddr);
		//EV<<"Purged list for "<<macAddr<<endl;
	}

	// delete entry from sendList
	//EV<<"sendList size before "<<sendList.size()<<endl;
	sendList.erase(macAddr);
	//EV<<"sendList size after "<<sendList.size()<<endl;
	if (sendList.empty())
	    EV<<"sendList purged"<<endl;

	//EV<<"deleteFrame() finished for addr "<<f->getDestAddr()<<endl;
	delete f;
}

void Mac80211a::appendFrame(Mac80211Pkt* f) {
    if (f->getDestAddr()!=L2BROADCAST)
    	fromUpperLayerByDest[f->getDestAddr()].push_back(f);
    else
    	EV<<"broadcast frame not appended in fromUpperLayerByDest"<<endl;
    fromUpperLayer.push_back(f);
}

int Mac80211a::getMode(double snr) {
    snr = snr * SYMBOL_TIME; // TODO check
    if (snr >= snrThresholds[7])
    	return 7;
    else if (snr >= snrThresholds[6])
    	return 6;
    else if (snr >= snrThresholds[5])
    	return 5;
    else if (snr >= snrThresholds[4])
    	return 4;
    else if (snr >= snrThresholds[3])
    	return 3;
    else if (snr >= snrThresholds[2])
    	return 2;
    else if (snr >= snrThresholds[1])
    	return 1;
    else 
    	return 0;
}	

/** @brief removes frame from sendList and destList (this extended cycle) */
void Mac80211a::cancelFrame(int dest) {
	EV<<"cancelFrame() called with param "<<dest<<endl;
	eraseFromDestList(dest);
	sendList.erase(dest);
}

void Mac80211a::eraseFromDestList(int dest) {
	EV<<"eraseFromDestList() called with param "<<dest<<endl;
	DestAddrList::iterator it = destList.begin();
	while ((it!=destList.end()) && (*it!=dest))
		it++;
	if (*it==dest)
	    destList.erase(it);
	else
	    EV<<"eraseFromDestList(): could not delete "<<dest<<", not in list"<<endl;
}

void Mac80211a::sendToAssignment(BasicAssignmentMessage* m) {
	send(m,toAssignment);
}

void Mac80211a::removeFromList(int dest) {
	BasicAssignmentMessage* m = new BasicAssignmentMessage();
	m->setKind(REMOVE_FROM_MODELIST);
	m->setMacAddr(dest);
	sendToAssignment(m);
}

void Mac80211a::clearAssignmentList() {
	BasicAssignmentMessage* m = new BasicAssignmentMessage();
	m->setKind(CLEAR_MODELIST);
	sendToAssignment(m);
}

void Mac80211a::requestAssignment() {
	BasicAssignmentMessage* m = new BasicAssignmentMessage();
	m->setKind(CALC_ASSIGNMENT);
	sendToAssignment(m);
}

long Mac80211a::getOFDMABitRate(int addr) {
/*
 *   6000000, // mode 1
 *   9000000, // mode 2
 *  12000000, // mode 3
 *  18000000, // mode 4
 *  24000000, // mode 5
 *  36000000, // mode 6
 *  48000000, // mode 7
 *  54000000, // mode 8
 */
	long rate = 0;
	for (int i=0;i<48;i++)
		if (myAssignment.entry[i].dest == addr) 
			rate+=SUBBAND_BITRATES_80211a[myAssignment.entry[i].mode];
	return rate;
}

void Mac80211a::recordModesToSink(int addr) {
	int count[8];
	for (int i=0;i<8;i++) 
		count[i]=0;
	for (int i=0;i<48;i++)
		if (myAssignment.entry[i].dest == addr) 
			count[myAssignment.entry[i].mode]++;
	for (int i=0;i<8;i++) 
		if (count[i])
			RECORD_SELECTED_MODE(i,count[i]);
}

void Mac80211a::recordToSink(int kind, double doubleVal) {
    SinkMessage* s = new SinkMessage("sinkmsg");
    s->setKind(kind);
    s->setDoubleValue(doubleVal);
    send(s,toSink);
}

void Mac80211a::recordToSink(int kind, int longVal) {
    SinkMessage* s = new SinkMessage("sinkmsg");
    s->setKind(kind);
    s->setLongValue(longVal);
    send(s,toSink);
}

void Mac80211a::recordToSink(int kind, long longVal) {
    SinkMessage* s = new SinkMessage("sinkmsg");
    s->setKind(kind);
    s->setLongValue(longVal);
    send(s,toSink);
}

void Mac80211a::recordToSink(int kind, int idx, int longVal) {
    SinkMessage* s = new SinkMessage("sinkmsg");
    s->setKind(kind);
    s->setIndex(idx);
    s->setLongValue(longVal);
    send(s,toSink);
}

int Mac80211a::getModeFromBitrate(long bitrate) {
    int i=0;
    while ((BITRATES_80211a[i]!=bitrate) && (i<8))
    	i++;
    if (BITRATES_80211a[i]==bitrate)
    	return i;
    else
        error("No correct IEEE 802.11a bitrate given");
}

bool Mac80211a::eRtsForMe(Mac80211Pkt* f) {
    DestAddrList l;
    DestAddrList::iterator it;
    bool eFound = false;
    if (f->kind() == ERTS) {
    l = f->getDestAddrList();
    for (it=l.begin();it!=l.end();it++) {
	if (*it == myMacAddr)
	eFound = true;
    }
    }
    return eFound;
}
