/*
 *  LMACLayer.cc
 *
 *
 *  Created by Anna Foerster on 10/10/08.
 *  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
 *
 *  Converted to OMNeT++ 4 by Rudolf Hornig
 *  Converted to MiXiM by Kapourniotis Theodoros
 */

#include "LMacLayer.h"

#include "FWMath.h"
#include "MacToPhyInterface.h"
#include "LMacPkt_m.h"

Define_Module( LMacLayer )

#define myId (getParentModule()->getParentModule()->getId()-4)

const LAddress::L2Type LMacLayer::LMAC_NO_RECEIVER = LAddress::L2Type(-2);
const LAddress::L2Type LMacLayer::LMAC_FREE_SLOT   = LAddress::L2BROADCAST;
/**
 * Initialize the of the omnetpp.ini variables in stage 1. In stage
 * two subscribe to the RadioState.
 */
void LMacLayer::initialize(int stage)
{
    BaseMacLayer::initialize(stage);

    if (stage == 0) {

        queueLength = par("queueLength");
        slotDuration = par("slotDuration");
        bitrate = par("bitrate");
		headerLength = par("headerLength");
		coreEV << "headerLength is: " << headerLength << endl;
        numSlots = par("numSlots");
		// the first N slots are reserved for mobile nodes to be able to function normally
		reservedMobileSlots = par("reservedMobileSlots");
		txPower = par("txPower");

        droppedPacket.setReason(DroppedPacket::NONE);
        nicId = getParentModule()->getId();
        debugEV << "My Mac address is" << myMacAddr << " and my Id is " << myId << endl;


        macState = INIT;

		slotChange = new cOutVector("slotChange");

		// how long does it take to send/receive a control packet
		controlDuration = ((double)headerLength + (double)numSlots + 16) / (double)bitrate;
		coreEV << "Control packets take : " << controlDuration << " seconds to transmit\n";
    }

    else if(stage == 1) {
        //int channel;
        //channel = hasPar("defaultChannel") ? par("defaultChannel") : 0;

        debugEV << "queueLength = " << queueLength
           << " slotDuration = " << slotDuration
		   << " controlDuration = " << controlDuration
		   << " numSlots = " << numSlots
           << " bitrate = " << bitrate << endl;

		timeout = new cMessage("timeout");
		timeout->setKind(LMAC_TIMEOUT);

		sendData = new cMessage("sendData");
		sendData->setKind(LMAC_SEND_DATA);

		wakeup = new cMessage("wakeup");
		wakeup->setKind(LMAC_WAKEUP);

		initChecker = new cMessage("setup phase");
		initChecker->setKind(LMAC_SETUP_PHASE_END);

		checkChannel = new cMessage("checkchannel");
		checkChannel->setKind(LMAC_CHECK_CHANNEL);

		start_lmac = new cMessage("start_lmac");
		start_lmac->setKind(LMAC_START_LMAC);

		send_control = new cMessage("send_control");
		send_control->setKind(LMAC_SEND_CONTROL);

		scheduleAt(0.0, start_lmac);


    }
}

LMacLayer::~LMacLayer() {
	delete slotChange;
	cancelAndDelete(timeout);
	cancelAndDelete(wakeup);
	cancelAndDelete(checkChannel);
	cancelAndDelete(sendData);
	cancelAndDelete(initChecker);
	cancelAndDelete(start_lmac);
	cancelAndDelete(send_control);

	MacQueue::iterator it;
    for(it = macQueue.begin(); it != macQueue.end(); ++it) {
        delete (*it);
    }
    macQueue.clear();
}

void LMacLayer::finish() {
}

/**
 * Check whether the queue is not full: if yes, print a warning and drop the packet.
 * Sending of messages is automatic.
 */
void LMacLayer::handleUpperMsg(cMessage *msg)
{
    LMacPkt *mac = static_cast<LMacPkt *>(encapsMsg(msg));

    // message has to be queued if another message is waiting to be send
    // or if we are already trying to send another message

    if (macQueue.size() <= queueLength) {
        macQueue.push_back(mac);
	debugEV << "packet put in queue\n  queue size: " << macQueue.size() << " macState: " << macState
	    << "; mySlot is " << mySlot << "; current slot is " << currSlot << endl;;

    }
    else {
        // queue is full, message has to be deleted
        debugEV << "New packet arrived, but queue is FULL, so new packet is deleted\n";
        mac->setName("MAC ERROR");
        mac->setKind(PACKET_DROPPED);
        sendControlUp(mac);
        droppedPacket.setReason(DroppedPacket::QUEUE);
        emit(catDroppedPacket, &droppedPacket);
		debugEV <<  "ERROR: Queue is full, forced to delete.\n";
    }
}

/**
 * Handle self messages:
 * LMAC_SETUP_PHASE_END: end of setup phase. Change slot duration to normal and start sending data packets. The slots of the nodes should be stable now.
 * LMAC_SEND_DATA: send the data packet.
 * LMAC_CHECK_CHANNEL: check the channel in own slot. If busy, change the slot. If not, send a control packet.
 * LMAC_WAKEUP: wake up the node and either check the channel before sending a control packet or wait for control packets.
 * LMAC_TIMEOUT: go back to sleep after nothing happened.
 */
void LMacLayer::handleSelfMsg(cMessage *msg)
{
	switch (macState)
	{
	case INIT:
		if (msg->getKind() == LMAC_START_LMAC)
		{
			// the first 5 full slots we will be waking up every controlDuration to setup the network first
			// normal packets will be queued, but will be send only after the setup phase
			scheduleAt(slotDuration*5*numSlots, initChecker);
			coreEV << "Startup time =" << slotDuration*5*numSlots << endl;

			debugEV << "Scheduling the first wakeup at : " << slotDuration << endl;

			scheduleAt(slotDuration, wakeup);

			for (int i = 0; i < numSlots; i++)
			{
				occSlotsDirect[i] = LMAC_FREE_SLOT;
				occSlotsAway[i]   = LMAC_FREE_SLOT;
			}

			if (myId >= reservedMobileSlots)
				mySlot = ((int) getParentModule()->getParentModule()->getId() )% (numSlots - reservedMobileSlots);
			else
				mySlot = myId;
			//occSlotsDirect[mySlot] = myMacAddr;
			//occSlotsAway[mySlot] = myMacAddr;
			currSlot = 0;

			debugEV << "ID: " << getParentModule()->getParentModule()->getId() << ". Picked random slot: " << mySlot << endl;

			macState=SLEEP;
			debugEV << "Old state: INIT, New state: SLEEP" << endl;
			SETUP_PHASE = true;
		}
		else {
			EV << "Unknown packet" << msg->getKind() <<  "in state" << macState << endl;
			delete msg;
		}
		break;

	case SLEEP:
		if(msg->getKind() == LMAC_WAKEUP)
		{
			currSlot++;
			currSlot %= numSlots;
			debugEV << "New slot starting - No. " << currSlot << ", my slot is " << mySlot << endl;

			if (mySlot == currSlot)
			{
				debugEV << "Waking up in my slot. Switch to RECV first to check the channel.\n";
				phy->setRadioState(Radio::RX);
				macState = CCA;
				debugEV << "Old state: SLEEP, New state: CCA" << endl;

				double small_delay = controlDuration*dblrand();
				scheduleAt(simTime()+small_delay, checkChannel);
				debugEV << "Checking for channel for " << small_delay << " time.\n";
			}
			else
			{
				debugEV << "Waking up in a foreign slot. Ready to receive control packet.\n";
				phy->setRadioState(Radio::RX);
				macState = WAIT_CONTROL;
				debugEV << "Old state: SLEEP, New state: WAIT_CONTROL" << endl;
				if (!SETUP_PHASE)	//in setup phase do not sleep
					scheduleAt(simTime()+2.f*controlDuration, timeout);
			}
			if (SETUP_PHASE)
			{
				scheduleAt(simTime()+2.f*controlDuration, wakeup);
				debugEV << "setup phase slot duration:" << 2.f*controlDuration << "while controlduration is" << controlDuration << endl;
			}
			else
				scheduleAt(simTime()+slotDuration, wakeup);
		}
		else if(msg->getKind() == LMAC_SETUP_PHASE_END)
		{
			debugEV << "Setup phase end. Start normal work at the next slot.\n";
			if (wakeup->isScheduled())
				cancelEvent(wakeup);

			scheduleAt(simTime()+slotDuration, wakeup);

			SETUP_PHASE = false;
		}
		else
		{
			EV << "Unknown packet" << msg->getKind() <<  "in state" << macState << endl;
			delete msg;
		}
		break;

	case CCA:
		if(msg->getKind() == LMAC_CHECK_CHANNEL)
		{
			// if the channel is clear, get ready for sending the control packet
			coreEV << "Channel is free, so let's prepare for sending.\n";

			phy->setRadioState(Radio::TX);
			macState = SEND_CONTROL;
			debugEV << "Old state: CCA, New state: SEND_CONTROL" << endl;

		}
		else if(msg->getKind() == LMAC_CONTROL)
		{
			LMacPkt *const          mac  = static_cast<LMacPkt *>(msg);
			const LAddress::L2Type& dest = mac->getDestAddr();
			debugEV << " I have received a control packet from src " << mac->getSrcAddr() << " and dest " << dest << ".\n";
			bool collision = false;
			// if we are listening to the channel and receive anything, there is a collision in the slot.
			if (checkChannel->isScheduled())
			{
				cancelEvent(checkChannel);
				collision = true;
			}

			for (int s = 0; s < numSlots; s++) {
				occSlotsAway[s] = mac->getOccupiedSlots(s);
				debugEV << "Occupied slot " << s << ": " << occSlotsAway[s] << endl;
				debugEV << "Occupied direct slot " << s << ": " << occSlotsDirect[s] << endl;
			}

			if (mac->getMySlot() >-1)
			{
				// check first whether this address didn't have another occupied slot and free it again
				for (int i=0; i < numSlots; i++)
				{
					if (occSlotsDirect[i] == mac->getSrcAddr())
						occSlotsDirect[i] = LMAC_FREE_SLOT;
					if (occSlotsAway[i] == mac->getSrcAddr())
						occSlotsAway[i] = LMAC_FREE_SLOT;
				}
				occSlotsAway[mac->getMySlot()]   = mac->getSrcAddr();
				occSlotsDirect[mac->getMySlot()] = mac->getSrcAddr();
			}
			collision = collision || (mac->getMySlot() == mySlot);
			if (((mySlot > -1) && (mac->getOccupiedSlots(mySlot) > LMAC_FREE_SLOT) && (mac->getOccupiedSlots(mySlot) != myMacAddr)) || collision)
			{
				debugEV << "My slot is taken by " << mac->getOccupiedSlots(mySlot) << ". I need to change it.\n";
				findNewSlot();
				debugEV << "My new slot is " << mySlot << endl;
			}
			if (mySlot < 0)
			{
				debugEV << "I don;t have a slot - try to find one.\n";
				findNewSlot();
			}

			if(dest == myMacAddr || LAddress::isL2Broadcast(dest))
			{
				debugEV << "I need to stay awake.\n";
				if (timeout->isScheduled())
					cancelEvent(timeout);
				macState=WAIT_DATA;
				debugEV << "Old state: CCA, New state: WAIT_DATA" << endl;
			}
			else
			{
				debugEV << "Incoming data packet not for me. Going back to sleep.\n";
				macState = SLEEP;
				debugEV << "Old state: CCA, New state: SLEEP" << endl;
				phy->setRadioState(Radio::SLEEP);
				if (timeout->isScheduled())
					cancelEvent(timeout);
			}
			delete mac;
		}
		//probably it never happens
		else if(msg->getKind() == LMAC_DATA)
		{
			LMacPkt *const          mac  = static_cast<LMacPkt *>(msg);
			const LAddress::L2Type& dest = mac->getDestAddr();
			//bool collision = false;
			// if we are listening to the channel and receive anything, there is a collision in the slot.
			if (checkChannel->isScheduled())
			{
				cancelEvent(checkChannel);
				//collision = true;
			}
			debugEV << " I have received a data packet.\n";
			if(dest == myMacAddr || LAddress::isL2Broadcast(dest))
			{
				debugEV << "sending pkt to upper...\n";
				sendUp(decapsMsg(mac));
			}
			else {
				debugEV << "packet not for me, deleting...\n";
				delete mac;
			}
			// in any case, go back to sleep
			macState = SLEEP;
			debugEV << "Old state: CCA, New state: SLEEP" << endl;
			phy->setRadioState(Radio::SLEEP);
		}
		else if(msg->getKind() == LMAC_SETUP_PHASE_END)
		{
			debugEV << "Setup phase end. Start normal work at the next slot.\n";
			if (wakeup->isScheduled())
				cancelEvent(wakeup);

			scheduleAt(simTime()+slotDuration, wakeup);

			SETUP_PHASE = false;
		}
		else
		{
			EV << "Unknown packet" << msg->getKind() <<  "in state" << macState << endl;
			delete msg;
		}
		break;

	case WAIT_CONTROL:
		if(msg->getKind() == LMAC_TIMEOUT)
		{
			debugEV << "Control timeout. Go back to sleep.\n";
			macState = SLEEP;
			debugEV << "Old state: WAIT_CONTROL, New state: SLEEP" << endl;
			phy->setRadioState(Radio::SLEEP);
		}
		else if(msg->getKind() == LMAC_CONTROL)
		{
			LMacPkt *const          mac  = static_cast<LMacPkt *>(msg);
			const LAddress::L2Type& dest = mac->getDestAddr();
			debugEV << " I have received a control packet from src " << mac->getSrcAddr() << " and dest " << dest << ".\n";

			bool collision = false;

			// check first the slot assignment
			// copy the current slot assignment

			for (int s = 0; s < numSlots; s++)
			{
				occSlotsAway[s] = mac->getOccupiedSlots(s);
				debugEV << "Occupied slot " << s << ": " << occSlotsAway[s] << endl;
				debugEV << "Occupied direct slot " << s << ": " << occSlotsDirect[s] << endl;
			}

			if (mac->getMySlot() >-1)
			{
				// check first whether this address didn't have another occupied slot and free it again
				for (int i=0; i < numSlots; i++)
				{
					if (occSlotsDirect[i] == mac->getSrcAddr())
						occSlotsDirect[i] = LMAC_FREE_SLOT;
					if (occSlotsAway[i] == mac->getSrcAddr())
						occSlotsAway[i] = LMAC_FREE_SLOT;
				}
				occSlotsAway[mac->getMySlot()]   = mac->getSrcAddr();
				occSlotsDirect[mac->getMySlot()] = mac->getSrcAddr();
			}

			collision = collision || (mac->getMySlot() == mySlot);
			if (((mySlot > -1) && (mac->getOccupiedSlots(mySlot) > LMAC_FREE_SLOT) && (mac->getOccupiedSlots(mySlot) != myMacAddr)) || collision)
			{
				debugEV << "My slot is taken by " << mac->getOccupiedSlots(mySlot) << ". I need to change it.\n";
				findNewSlot();
				debugEV << "My new slot is " << mySlot << endl;
			}
			if (mySlot < 0)
			{
				debugEV << "I don;t have a slot - try to find one.\n";
				findNewSlot();
			}

			if(dest == myMacAddr || LAddress::isL2Broadcast(dest))
			{
				debugEV << "I need to stay awake.\n";
				macState=WAIT_DATA;
				debugEV << "Old state: WAIT_CONTROL, New state: WAIT_DATA" << endl;
				if (timeout->isScheduled())
					cancelEvent(timeout);
			}
			else
			{
				debugEV << "Incoming data packet not for me. Going back to sleep.\n";
				macState = SLEEP;
				debugEV << "Old state: WAIT_CONTROL, New state: SLEEP" << endl;
				phy->setRadioState(Radio::SLEEP);
				if (timeout->isScheduled())
					cancelEvent(timeout);
			}
			delete mac;
		}
		else if ((msg->getKind() == LMAC_WAKEUP))
		{
			if (SETUP_PHASE == true)
				debugEV << "End of setup-phase slot" << endl;
			else
				debugEV << "Very unlikely transition";

			macState = SLEEP;
			debugEV << "Old state: WAIT_DATA, New state: SLEEP" << endl;
			scheduleAt(simTime(), wakeup);

		}
		else if (msg->getKind() == LMAC_SETUP_PHASE_END)
		{
			debugEV << "Setup phase end. Start normal work at the next slot.\n";
			if (wakeup->isScheduled())
				cancelEvent(wakeup);

			scheduleAt(simTime()+slotDuration, wakeup);

			SETUP_PHASE = false;
		}
		else
		{
			EV << "Unknown packet" << msg->getKind() <<  "in state" << macState << endl;
			delete msg;
		}

		break;

	case SEND_CONTROL:

		if(msg->getKind() == LMAC_SEND_CONTROL)
		{
			// send first a control message, so that non-receiving nodes can switch off.
			coreEV << "Sending a control packet.\n";
			LMacPkt* control = new LMacPkt();
			control->setKind(LMAC_CONTROL);
			if ((macQueue.size() > 0) && !SETUP_PHASE)
				control->setDestAddr((macQueue.front())->getDestAddr());
			else
				control->setDestAddr(LMAC_NO_RECEIVER);

			control->setSrcAddr(myMacAddr);
			control->setMySlot(mySlot);
			control->setBitLength(headerLength + numSlots);
			control->setOccupiedSlotsArraySize(numSlots);
			for (int i = 0; i < numSlots; i++)
				control->setOccupiedSlots(i, occSlotsDirect[i]);

			attachSignal(control);
			sendDown(control);
			if ((macQueue.size() > 0) && (!SETUP_PHASE))
				scheduleAt(simTime()+controlDuration, sendData);
		}
		else if(msg->getKind() == LMAC_SEND_DATA)
		{
			// we should be in our own slot and the control packet should be already sent. receiving neighbors should wait for the data now.
			if (currSlot != mySlot)
			{
				debugEV << "ERROR: Send data message received, but we are not in our slot!!! Repair.\n";
				phy->setRadioState(Radio::SLEEP);
				if (timeout->isScheduled())
					cancelEvent(timeout);
				return;
			}
			LMacPkt* data = macQueue.front()->dup();
			data->setKind(LMAC_DATA);
			data->setMySlot(mySlot);
			data->setOccupiedSlotsArraySize(numSlots);
			for (int i = 0; i < numSlots; i++)
				data->setOccupiedSlots(i, occSlotsDirect[i]);

			attachSignal(data);
			coreEV << "Sending down data packet\n";
			sendDown(data);
			delete macQueue.front();
			macQueue.pop_front();
			macState = SEND_DATA;
			debugEV << "Old state: SEND_CONTROL, New state: SEND_DATA" << endl;
		}
		else if(msg->getKind() == LMAC_SETUP_PHASE_END)
		{
			debugEV << "Setup phase end. Start normal work at the next slot.\n";
			if (wakeup->isScheduled())
				cancelEvent(wakeup);

			scheduleAt(simTime()+slotDuration, wakeup);

			SETUP_PHASE = false;
		}
		else
		{
			EV << "Unknown packet" << msg->getKind() <<  "in state" << macState << endl;
			delete msg;
		}
		break;

	case SEND_DATA:
		if(msg->getKind() == LMAC_WAKEUP)
		{
			error("I am still sending a message, while a new slot is starting!\n");
		}
		else
		{
			EV << "Unknown packet" << msg->getKind() <<  "in state" << macState << endl;
			delete msg;
		}
		break;

	case WAIT_DATA:
		if(msg->getKind() == LMAC_DATA)
		{
			LMacPkt *const          mac  = static_cast<LMacPkt *>(msg);
			const LAddress::L2Type& dest = mac->getDestAddr();

			debugEV << " I have received a data packet.\n";
			if(dest == myMacAddr || LAddress::isL2Broadcast(dest))
			{
				debugEV << "sending pkt to upper...\n";
				sendUp(decapsMsg(mac));
			}
			else {
				debugEV << "packet not for me, deleting...\n";
				delete mac;
			}
			// in any case, go back to sleep
			macState = SLEEP;
			debugEV << "Old state: WAIT_DATA, New state: SLEEP" << endl;
			phy->setRadioState(Radio::SLEEP);
			if (timeout->isScheduled())
				cancelEvent(timeout);
		}
		else if(msg->getKind() == LMAC_WAKEUP)
		{
			macState = SLEEP;
			debugEV << "Unlikely transition. Old state: WAIT_DATA, New state: SLEEP" << endl;
			scheduleAt(simTime(), wakeup);
		}
		else
		{
			EV << "Unknown packet" << msg->getKind() <<  "in state" << macState << endl;
			delete msg;
		}
		break;
	default:
		opp_error("Unknown mac state: %d", macState);
		break;
	}
}

/**
 * Handle LMAC control packets and data packets. Recognize collisions, change own slot if necessary and remember who is using which slot.
 */
void LMacLayer::handleLowerMsg(cMessage *msg)
{
	// simply pass the massage as self message, to be processed by the FSM.
	handleSelfMsg(msg);
}


/**
 * Handle transmission over messages: send the data packet or don;t do anyhting.
 */
void LMacLayer::handleLowerControl(cMessage *msg)
{
	if(msg->getKind() == MacToPhyInterface::TX_OVER)
	{
		// if data is scheduled for transfer, don;t do anything.
		if (sendData->isScheduled())
		{
			debugEV << " transmission of control packet over. data transfer will start soon." << endl;
			delete msg;
			return;
		}
		else
		{
			debugEV << " transmission over. nothing else is scheduled, get back to sleep." << endl;
			macState = SLEEP;
			debugEV << "Old state: ?, New state: SLEEP" << endl;
			phy->setRadioState(Radio::SLEEP);
			if (timeout->isScheduled())
				cancelEvent(timeout);
		}
    }

	else if(msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER)
	{
	   	// we just switched to TX after CCA, so simply send the first sendPremable self message
	   	if ((macState == SEND_CONTROL) && (phy->getRadioState() == Radio::TX))
	   	{
	   		scheduleAt(simTime(), send_control);
	   	}

	}

    else {
        EV << "control message with wrong kind -- deleting\n";
    }
    delete msg;

}


/**
 * Try to find a new slot after collision. If not possible, set own slot to -1 (not able to send anything)
 */
void LMacLayer::findNewSlot()
{
	// pick a random slot at the beginning and schedule the next wakeup
	// free the old one first
	int counter = 0;

	mySlot = intrand((numSlots - reservedMobileSlots));
	while ((occSlotsAway[mySlot] != LMAC_FREE_SLOT) && (counter < (numSlots - reservedMobileSlots)))
	{
		counter++;
		mySlot--;
		if (mySlot < 0)
			mySlot = (numSlots - reservedMobileSlots)-1;
	}
	if (occSlotsAway[mySlot] != LMAC_FREE_SLOT)
	{
		EV << "ERROR: I cannot find a free slot. Cannot send data.\n";
		mySlot = -1;
	}
	else
	{
		EV << "ERROR: My new slot is : " << mySlot << endl;
	}
	EV << "ERROR: I needed to find new slot\n";
	slotChange->recordWithTimestamp(simTime(), getParentModule()->getParentModule()->getId()-4);
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all needed
 * header fields.
 */
MacPkt *LMacLayer::encapsMsg(cMessage * msg)
{

    LMacPkt *pkt = new LMacPkt(msg->getName(), msg->getKind());
    pkt->setBitLength(headerLength);

    // copy dest address from the Control Info attached to the network
    // mesage by the network layer
    cObject *const cInfo = msg->removeControlInfo();

    debugEV << "CInfo removed, mac addr=" << getUpperDestinationFromControlInfo(cInfo) << endl;
    pkt->setDestAddr(getUpperDestinationFromControlInfo(cInfo));

    //delete the control info
    delete cInfo;

    //set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(myMacAddr);

    //encapsulate the network packet
    pkt->encapsulate(check_and_cast<cPacket *>(msg));
    debugEV <<"pkt encapsulated\n";

    return pkt;

}

void LMacLayer::attachSignal(MacPkt *macPkt)
{
	//calc signal duration
	simtime_t duration = macPkt->getBitLength() / bitrate;
	//create signal
	setDownControlInfo(macPkt, createSignal(simTime(), duration, txPower, bitrate));
}

