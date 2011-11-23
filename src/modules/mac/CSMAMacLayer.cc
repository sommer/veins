

#include "CSMAMacLayer.h"

#include "FWMath.h"
#include "BaseConnectionManager.h"
#include "MacToPhyInterface.h"
#include "MacPkt_m.h"
#include "PhyUtils.h"

Define_Module( CSMAMacLayer )

/**
 * Initialize the of the omnetpp.ini variables in stage 1. In stage
 * two subscribe to the RadioState.
 */
void CSMAMacLayer::initialize(int stage)
{
    BaseMacLayer::initialize(stage);

    if (stage == 0) {

        queueLength = hasPar("queueLength") 	? par("queueLength").longValue()	 : 10;
        //busyRSSI = hasPar("busyRSSI") 			? par("busyRSSI").doubleValue() 	 : -90;
        slotDuration = hasPar("slotDuration") 	? par("slotDuration").doubleValue()	 : 0.1;
        difs = hasPar("difs") 					? par("difs").doubleValue()			 : 0.001;
        maxTxAttempts = hasPar("maxTxAttempts") ? par("maxTxAttempts").longValue()	 : 7;
        bitrate = hasPar("bitrate") 			? par("bitrate").doubleValue()		 : 10000;
        txPower = hasPar("txPower")				? par("txPower").doubleValue()		 : 50;
        initialCW = hasPar("contentionWindow") 	? par("contentionWindow").longValue(): 31;

        macState = RX;

        //droppedPacket.setReason(DroppedPacket::NONE);

        //nicId = getParentModule()->getId();

        // initialize the timer
        backoffTimer = new cMessage("backoff");
        minorMsg = new cMessage("minClear");

        nbBackoffs = 0;
		backoffValues = 0;
		nbTxFrames = 0;

        txAttempts = 0;
    }
    else if(stage == 1) {
    	BaseConnectionManager* cc = getConnectionManager();

    	if(cc->hasPar("pMax") && txPower > cc->par("pMax").doubleValue())
            opp_error("TranmitterPower can't be bigger than pMax in ConnectionManager! "
            		  "Please adjust your omnetpp.ini file accordingly.");

    	if(phy->getRadioState() != Radio::RX) {
    		opp_error("Initial radio state isn't RX but CSMAMacLayer"
    				  " assumes that the NIC starts in RX state.");
    	}

    	debugEV << "queueLength = " << queueLength
           //<< " busyRSSI = " << busyRSSI
           << " slotDuration = " << slotDuration
           << " difs = " << difs
           << " maxTxAttempts = " << maxTxAttempts
           << " bitrate = " << bitrate
           << " contentionWindow = " << initialCW << endl;

        //busyRSSI = FWMath::dBm2mW(busyRSSI);
    }
}

CSMAMacLayer::~CSMAMacLayer() {
	cancelAndDelete(backoffTimer);
	cancelAndDelete(minorMsg);

    for(MacQueue::iterator it = macQueue.begin();
		it != macQueue.end(); ++it)
    {
        delete (*it);
    }
    macQueue.clear();
}

void CSMAMacLayer::finish() {
	recordScalar("nbBackoffs", nbBackoffs);
	recordScalar("backoffDurations", backoffValues);
	recordScalar("nbTxFrames", nbTxFrames);


}

/**
 * First it has to be checked whether a frame is currently being
 * transmitted or waiting to be transmitted. If so the newly arrived
 * message is stored in a queue. If there is no queue or it is full
 * print a warning.
 *
 * Before transmitting a frame it is tested whether the channel
 * is busy at the moment or not. If the channel is busy, a short
 * random time will be generated and the MacPkt is buffered for this
 * time, before a next attempt to send the packet is started.
 *
 * If channel is idle the frame will be transmitted immediately.
 */
void CSMAMacLayer::handleUpperMsg(cMessage *msg)
{
	cPacket* pkt = static_cast<cPacket*>(msg);

    //MacPkt *mac = encapsMsg(pkt);

    // message has to be queued if another message is waiting to be send
    // or if we are already trying to send another message

    if (macQueue.size() <= queueLength)
    {
        macQueue.push_back(pkt);
        debugEV 	<< "packet putt in queue\n  queue size:" << macQueue.size() << " macState:" << macState
			<< " (RX=" << RX << ") is scheduled:" << backoffTimer->isScheduled() << endl;;

        if((macQueue.size() == 1) && (macState == RX) && !backoffTimer->isScheduled()) {
            scheduleBackoff();
        }
    }
    else {
        // queue is full, message has to be deleted
    	EV << "New packet arrived, but queue is FULL, so new packet is deleted\n";
        MacPkt* mac = encapsMsg(pkt);
        mac->setName("MAC ERROR");
        mac->setKind(PACKET_DROPPED);
        sendControlUp(mac);

        //TODO: send packet drop bb info
        //droppedPacket.setReason(DroppedPacket::QUEUE);
        //emitItem(catDroppedPacket, &droppedPacket);
    }
}

/**
 * After the timer expires try to retransmit the message by calling
 * handleUpperMsg again.
 */
void CSMAMacLayer::handleSelfMsg(cMessage *msg)
{
    if(msg == backoffTimer) {
    	debugEV << "backoffTimer ";

        if(macState == RX) {
        	debugEV << " RX ";

            if(macQueue.size() != 0) {
            	macState = CCA;

                if(phy->getRadioState() == Radio::RX) {

                    if(phy->getChannelState().isIdle()) {
                    	debugEV << " idle ";
                        scheduleAt(simTime()+difs, minorMsg);
                    }
                    else{
                    	macState = RX;
                    	debugEV << " busy ";
                        scheduleBackoff();
                    }
                }
            }
        }
        else {
        	debugEV << "" << endl;
        	debugEV << "state=" << macState << "(TX="<<TX<<", CCA="<<CCA<<")\n";
            error("backoffTimer expired, MAC in wrong state");
        }
    }
    else if(msg == minorMsg) {
    	debugEV << " minorMsg ";

        //TODO: replace with channel sense request
        if((macState == CCA) && (phy->getRadioState() == Radio::RX)) {

            if(phy->getChannelState().isIdle()) {
            	debugEV << " idle -> to send ";
                macState = TX;
                phy->setRadioState(Radio::TX);
            }
            else {
            	debugEV << " busy -> backoff ";
                macState = RX;
                if(!backoffTimer->isScheduled())
                	scheduleBackoff();
            }
        }
        else {
            error("minClearTimer fired -- channel or mac in wrong state");
        }
    }
    debugEV << endl;
}


/**
 * Compare the address of this Host with the destination address in
 * frame. If they are equal or the frame is broadcast, we send this
 * frame to the upper layer. If not delete it.
 */
void CSMAMacLayer::handleLowerMsg(cMessage *msg)
{
    MacPkt*                 mac  = static_cast<MacPkt *>(msg);
    const LAddress::L2Type& dest = mac->getDestAddr();

    if(dest == myMacAddr || LAddress::isL2Broadcast(dest))
    {
    	debugEV << "sending pkt to upper...\n";
        sendUp(decapsMsg(mac));
    }
    else {
    	debugEV << "packet not for me, deleting...\n";
        delete mac;
    }

    if(!backoffTimer->isScheduled()) scheduleBackoff();
}

void CSMAMacLayer::handleLowerControl(cMessage *msg)
{
    if(msg->getKind() == MacToPhyInterface::TX_OVER) {
    	debugEV << " transmission over" << endl;
        macState = RX;
        phy->setRadioState(Radio::RX);
        txAttempts = 0;
        if(!backoffTimer->isScheduled()) scheduleBackoff();
    }
    else if(msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
    	if((macState == TX) && (phy->getRadioState() == Radio::TX)) {
    		debugEV << " radio switched to tx, sendDown packet" << endl;
            nbTxFrames++;
            sendDown(encapsMsg(macQueue.front()));
            macQueue.pop_front();
        }
    }
    else {
    	EV << "control message with wrong kind -- deleting\n";
    }
    delete msg;
}

void CSMAMacLayer::scheduleBackoff()
{
    if(backoffTimer->isScheduled()) {
        std::cerr << " is scheduled: MAC state "
                  << macState << " radio state : " << phy->getRadioState() << endl;
    }

    if(minorMsg->isScheduled()){
		cancelEvent( minorMsg );
		macState=RX;
	}

    if(txAttempts > maxTxAttempts) {
    	debugEV << " drop packet " << endl;

        cMessage *mac = encapsMsg(macQueue.front());
        mac->setName("MAC ERROR");
        mac->setKind(PACKET_DROPPED);
        txAttempts = 0;
        macQueue.pop_front();
        sendControlUp(mac);

        //TODO: send packet drop bb info
        //droppedPacket.setReason(DroppedPacket::CHANNEL);
        //emit(catDroppedPacket, &droppedPacket);
    }

    if(macQueue.size() != 0) {
    	debugEV << " schedule backoff " << endl;

        double slots = intrand(initialCW + txAttempts) + 1.0 + dblrand();
        double time = slots * slotDuration;

        txAttempts++;
        debugEV << " attempts so far: " << txAttempts  << " " << endl;

		nbBackoffs = nbBackoffs + 1;
		backoffValues = backoffValues + time;

        scheduleAt(simTime() + time, backoffTimer);
    }

}

MacPkt* CSMAMacLayer::encapsMsg(cPacket *pkt)
{
	MacPkt* macPkt = BaseMacLayer::encapsMsg(pkt);

	//calc signal duration
	simtime_t duration = macPkt->getBitLength() / bitrate;

	if(duration > slotDuration) {
		EV << "Warning: Sending packet " << pkt
		   << " - duration (" << duration
		   << " ) is bigger than the slot duration (" << slotDuration
		   <<")." << endl;
	}

	//create signal
	setDownControlInfo(macPkt, createSignal(simTime(), duration, txPower, bitrate));

	return macPkt;
}



