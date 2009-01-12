

#include "CSMAMacLayer.h"
#include "NicControlType.h"
#include "FWMath.h"
#include "MacToPhyControlInfo.h"

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
        busyRSSI = hasPar("busyRSSI") 			? par("busyRSSI").doubleValue() 	 : -90;
        slotDuration = hasPar("slotDuration") 	? par("slotDuration").doubleValue()	 : 0.1;
        difs = hasPar("difs") 					? par("difs").doubleValue()			 : 0.001;
        maxTxAttempts = hasPar("maxTxAttempts") ? par("maxTxAttempts").longValue()	 : 7;
        bitrate = hasPar("bitrate") 			? par("bitrate").doubleValue()		 : 10000;
        txPower = hasPar("txPower")				? par("txPower").doubleValue()		 : 50;
        initialCW = hasPar("contentionWindow") 	? par("contentionWindow").longValue(): 31;

        assert(phy->getRadioState() == Radio::RX);
        macState = RX;

        //droppedPacket.setReason(DroppedPacket::NONE);
        if(!getParentModule()->hasPar("id"))
        	error("Nic module has no id parameter!");

        nicId = getParentModule()->par("id");

        // initialize the timer
        backoffTimer = new cMessage("backoff");
        minorMsg = new cMessage("minClear");

        txAttempts = 0;
    }
    else if(stage == 1) {
    	//TODO: see what the old radio in mfw does with the bitrate
        //int channel;

        //channel = hasPar("defaultChannel") ? par("defaultChannel").longValue() : 0;
        //radio->setActiveChannel(channel);
        //radio->setBitrate(bitrate);

    	myMacAddr = nicId;

        EV << "queueLength = " << queueLength
           << " busyRSSI = " << busyRSSI
           << " slotDuration = " << slotDuration
           << " difs = " << difs
           << " maxTxAttempts = " << maxTxAttempts
           << " bitrate = " << bitrate
           << " contentionWindow = " << initialCW << endl;

        busyRSSI = FWMath::dBm2mW(busyRSSI);
    }
}


void CSMAMacLayer::finish() {
    if (!backoffTimer->isScheduled())
    	delete backoffTimer;
    if (!minorMsg->isScheduled())
    	delete minorMsg;

    for(MacQueue::iterator it = macQueue.begin();
		it != macQueue.end(); ++it)
    {
        delete (*it);
    }
    macQueue.clear();
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
		EV 	<< "packet putt in queue\n  queue size:" << macQueue.size() << " macState:" << macState
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
        mac->setKind(NicControlType::PACKET_DROPPED);
        sendControlUp(mac);

        //TODO: send packet drop bb info
        //droppedPacket.setReason(DroppedPacket::QUEUE);
        //utility->publishBBItem(catDroppedPacket, &droppedPacket, nicId);
    }
}

/**
 * After the timer expires try to retransmit the message by calling
 * handleUpperMsg again.
 */
void CSMAMacLayer::handleSelfMsg(cMessage *msg)
{
    if(msg == backoffTimer) {
        EV << "backoffTimer ";

        if(macState == RX) {
            EV << " RX ";

            if(macQueue.size() != 0) {
            	macState = CCA;

                if(phy->getRadioState() == Radio::RX) {
                	double rssi = phy->getChannelState().getRSSI();
                	EV << "rssi:" << rssi << " busyRSSI:" << busyRSSI << " ";

                    if(rssi < busyRSSI) {
                        EV << " idle ";
                        scheduleAt(simTime()+difs, minorMsg);
                    }
                    else{
                    	macState = RX;
                        EV << " busy ";
                        scheduleBackoff();
                    }
                }
            }
        }
        else {
        	EV << "" << endl;
        	EV << "state=" << macState << "(TX="<<TX<<", CCA="<<CCA<<")\n";
            error("backoffTimer expired, MAC in wrong state");
        }
    }
    else if(msg == minorMsg) {
        EV << " minorMsg ";

        //TODO: replace with channel sense request
        if((macState == CCA) && (phy->getRadioState() == Radio::RX)) {
        	double rssi = phy->getChannelState().getRSSI();
        	EV << "rssi:" << rssi << " busyRSSI:" << busyRSSI << " ";

            if(rssi < busyRSSI) {
                EV << " idle -> to send ";
                macState = TX;
                phy->setRadioState(Radio::TX);
            }
            else {
                EV << " busy -> backoff ";
                macState = RX;
                if(!backoffTimer->isScheduled())
                	scheduleBackoff();
            }
        }
        else {
            error("minClearTimer fired -- channel or mac in wrong state");
        }
    }
    EV << endl;
}


/**
 * Compare the address of this Host with the destination address in
 * frame. If they are equal or the frame is broadcast, we send this
 * frame to the upper layer. If not delete it.
 */
void CSMAMacLayer::handleLowerMsg(cMessage *msg)
{
    MacPkt *mac = static_cast<MacPkt *>(msg);
    int dest = mac->getDestAddr();

    if(dest == myMacAddr || dest == L2BROADCAST)
    {
        EV << "sending pkt to upper...\n";
        sendUp(decapsMsg(mac));
    }
    else {
        EV << "packet not for me, deleting...\n";
        delete mac;
    }

    if(!backoffTimer->isScheduled()) scheduleBackoff();
}

void CSMAMacLayer::handleLowerControl(cMessage *msg)
{
    if(msg->getKind() == MacToPhyInterface::TX_OVER) {
        EV << " transmission over" << endl;
        macState = RX;
        phy->setRadioState(Radio::RX);
        txAttempts = 0;
    }
    else if(msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
    	if((macState == TX) && (phy->getRadioState() == Radio::TX)) {
            EV << " radio switched to tx, sendDown packet" << endl;

            sendDown(encapsMsg(macQueue.front()));
            macQueue.pop_front();
        }
    }
    else {
        EV << "control message with wrong kind -- deleting\n";
    }
    delete msg;

    if(!backoffTimer->isScheduled()) scheduleBackoff();
}

void CSMAMacLayer::scheduleBackoff()
{
    if(backoffTimer->isScheduled()) {
        std::cerr << " is scheduled: MAC state "
                  << macState << " radio state : " << phy->getRadioState() << endl;
    }

    if(txAttempts > maxTxAttempts) {
        EV << " drop packet " << endl;

        cMessage *mac = encapsMsg(macQueue.front());
        mac->setName("MAC ERROR");
        mac->setKind(NicControlType::PACKET_DROPPED);
        txAttempts = 0;
        macQueue.pop_front();
        sendControlUp(mac);

        //TODO: send packet drop bb info
        //droppedPacket.setReason(DroppedPacket::CHANNEL);
        //bb->publishBBItem(catDroppedPacket, &droppedPacket, nicId);
    }

    if(macQueue.size() != 0) {
        EV << " schedule backoff " << endl;
        txAttempts++;
        EV << " attempts so far: " << txAttempts  << " " << endl;
        double slots = intrand(initialCW - txAttempts) + 2.0*dblrand();
        double time = std::max(slots, 1.0) * slotDuration;

		if(minorMsg->isScheduled()){
			cancelEvent( minorMsg );
			macState=RX;
		}

        scheduleAt(simTime() + time, backoffTimer);
    }

}

MacPkt* CSMAMacLayer::encapsMsg(cPacket *pkt)
{
	MacPkt* macPkt = BaseMacLayer::encapsMsg(pkt);

	//calc signal duration
	simtime_t duration = macPkt->getBitLength() / bitrate;
	//create signal
	Signal* s = createSignal(simTime(), duration, txPower, bitrate);

	//create and initialize control info
	MacToPhyControlInfo* ctrl = new MacToPhyControlInfo(s);

	macPkt->setControlInfo(ctrl);

	return macPkt;
}



