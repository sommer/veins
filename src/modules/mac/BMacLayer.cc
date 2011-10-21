//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "BMacLayer.h"

#include <cassert>

#include "FWMath.h"
#include "MacToPhyControlInfo.h"
#include "BaseArp.h"
#include "BaseConnectionManager.h"
#include "PhyUtils.h"
#include "MacPkt_m.h"
#include "MacToPhyInterface.h"

Define_Module( BMacLayer )

/**
 * Initialize method of BMacLayer. Init all parameters, schedule timers.
 */
void BMacLayer::initialize(int stage)
{
	BaseMacLayer::initialize(stage);

    if (stage == 0) {

		queueLength = hasPar("queueLength") ? par("queueLength") : 10;
		animation = hasPar("animation") ? par("animation") : true;
		slotDuration = hasPar("slotDuration") ? par("slotDuration") : 1;
		bitrate = hasPar("bitrate") ? par("bitrate") : 15360;
		headerLength = hasPar("headerLength") ? par("headerLength") : 10;
		checkInterval = hasPar("checkInterval") ? par("checkInterval") : 0.1;
		txPower = hasPar("txPower") ? par("txPower") : 50;
		useMacAcks = hasPar("useMACAcks") ? par("useMACAcks") : false;
		maxTxAttempts = hasPar("maxTxAttempts") ? par("maxTxAttempts") : 2;
		debugEV << "headerLength: " << headerLength << ", bitrate: " << bitrate << endl;

		stats = par("stats");
		nbTxDataPackets = 0;
		nbTxPreambles = 0;
		nbRxDataPackets = 0;
		nbRxPreambles = 0;
		nbMissedAcks = 0;
		nbRecvdAcks=0;
		nbDroppedDataPackets=0;
		nbTxAcks=0;

		txAttempts = 0;
		lastDataPktDestAddr = LAddress::L2BROADCAST;
		lastDataPktSrcAddr  = LAddress::L2BROADCAST;

		macState = INIT;

		// init the dropped packet info
		droppedPacket.setReason(DroppedPacket::NONE);
		nicId = getParentModule()->getId();
		WATCH(macState);
    }

    else if(stage == 1) {

		wakeup = new cMessage("wakeup");
		wakeup->setKind(BMAC_WAKE_UP);

		data_timeout = new cMessage("data_timeout");
		data_timeout->setKind(BMAC_DATA_TIMEOUT);
		data_timeout->setSchedulingPriority(100);

		data_tx_over = new cMessage("data_tx_over");
		data_tx_over->setKind(BMAC_DATA_TX_OVER);

		stop_preambles = new cMessage("stop_preambles");
		stop_preambles->setKind(BMAC_STOP_PREAMBLES);

		send_preamble = new cMessage("send_preamble");
		send_preamble->setKind(BMAC_SEND_PREAMBLE);

		ack_tx_over = new cMessage("ack_tx_over");
		ack_tx_over->setKind(BMAC_ACK_TX_OVER);

		cca_timeout = new cMessage("cca_timeout");
		cca_timeout->setKind(BMAC_CCA_TIMEOUT);
		cca_timeout->setSchedulingPriority(100);

		send_ack = new cMessage("send_ack");
		send_ack->setKind(BMAC_SEND_ACK);

		start_bmac = new cMessage("start_bmac");
		start_bmac->setKind(BMAC_START_BMAC);

		ack_timeout = new cMessage("ack_timeout");
		ack_timeout->setKind(BMAC_ACK_TIMEOUT);

		resend_data = new cMessage("resend_data");
		resend_data->setKind(BMAC_RESEND_DATA);
		resend_data->setSchedulingPriority(100);

		scheduleAt(0.0, start_bmac);
    }
}

BMacLayer::~BMacLayer()
{
	cancelAndDelete(wakeup);
	cancelAndDelete(data_timeout);
	cancelAndDelete(data_tx_over);
	cancelAndDelete(stop_preambles);
	cancelAndDelete(send_preamble);
	cancelAndDelete(ack_tx_over);
	cancelAndDelete(cca_timeout);
	cancelAndDelete(send_ack);
	cancelAndDelete(start_bmac);
	cancelAndDelete(ack_timeout);
	cancelAndDelete(resend_data);

	MacQueue::iterator it;
	for(it = macQueue.begin(); it != macQueue.end(); ++it)
	{
		delete (*it);
	}
	macQueue.clear();
}

void BMacLayer::finish()
{
    BaseMacLayer::finish();

    // record stats
    if (stats)
    {
    	recordScalar("nbTxDataPackets", nbTxDataPackets);
    	recordScalar("nbTxPreambles", nbTxPreambles);
    	recordScalar("nbRxDataPackets", nbRxDataPackets);
    	recordScalar("nbRxPreambles", nbRxPreambles);
    	recordScalar("nbMissedAcks", nbMissedAcks);
    	recordScalar("nbRecvdAcks", nbRecvdAcks);
    	recordScalar("nbTxAcks", nbTxAcks);
    	recordScalar("nbDroppedDataPackets", nbDroppedDataPackets);
    	//recordScalar("timeSleep", timeSleep);
    	//recordScalar("timeRX", timeRX);
    	//recordScalar("timeTX", timeTX);
    }
}

/**
 * Check whether the queue is not full: if yes, print a warning and drop the
 * packet. Then initiate sending of the packet, if the node is sleeping. Do
 * nothing, if node is working.
 */
void BMacLayer::handleUpperMsg(cMessage *msg)
{
	bool pktAdded = addToQueue(msg);
	if (!pktAdded)
		return;
	// force wakeup now
	if (wakeup->isScheduled() && (macState == SLEEP))
	{
		cancelEvent(wakeup);
		scheduleAt(simTime() + dblrand()*0.1f, wakeup);
	}
}

/**
 * Send one short preamble packet immediately.
 */
void BMacLayer::sendPreamble()
{
	MacPkt* preamble = new MacPkt();
	preamble->setSrcAddr(myMacAddr);
	preamble->setDestAddr(LAddress::L2BROADCAST);
	preamble->setKind(BMAC_PREAMBLE);
	preamble->setBitLength(headerLength);

	//attach signal and send down
	attachSignal(preamble);
	sendDown(preamble);
	nbTxPreambles++;
}

/**
 * Send one short preamble packet immediately.
 */
void BMacLayer::sendMacAck()
{
	MacPkt* ack = new MacPkt();
	ack->setSrcAddr(myMacAddr);
	ack->setDestAddr(lastDataPktSrcAddr);
	ack->setKind(BMAC_ACK);
	ack->setBitLength(headerLength);

	//attach signal and send down
	attachSignal(ack);
	sendDown(ack);
	nbTxAcks++;
	//endSimulation();
}



/**
 * Handle own messages:
 * BMAC_WAKEUP: wake up the node, check the channel for some time.
 * BMAC_CHECK_CHANNEL: if the channel is free, check whether there is something
 * in the queue and switch the radio to TX. When switched to TX, the node will
 * start sending preambles for a full slot duration. If the channel is busy,
 * stay awake to receive message. Schedule a timeout to handle false alarms.
 * BMAC_SEND_PREAMBLES: sending of preambles over. Next time the data packet
 * will be send out (single one).
 * BMAC_TIMEOUT_DATA: timeout the node after a false busy channel alarm. Go
 * back to sleep.
 */
void BMacLayer::handleSelfMsg(cMessage *msg)
{
	switch (macState)
	{
	case INIT:
		if (msg->getKind() == BMAC_START_BMAC)
		{
			debugEV << "State INIT, message BMAC_START, new state SLEEP" << endl;
			changeDisplayColor(BLACK);
			phy->setRadioState(Radio::SLEEP);
			macState = SLEEP;
			scheduleAt(simTime()+dblrand()*slotDuration, wakeup);
			return;
		}
		break;
	case SLEEP:
		if (msg->getKind() == BMAC_WAKE_UP)
		{
			debugEV << "State SLEEP, message BMAC_WAKEUP, new state CCA" << endl;
			scheduleAt(simTime() + checkInterval, cca_timeout);
			phy->setRadioState(Radio::RX);
			changeDisplayColor(GREEN);
			macState = CCA;
			return;
		}
		break;
	case CCA:
		if (msg->getKind() == BMAC_CCA_TIMEOUT)
		{
			// channel is clear
			// something waiting in eth queue?
			if (macQueue.size() > 0)
			{
				debugEV << "State CCA, message CCA_TIMEOUT, new state"
						  " SEND_PREAMBLE" << endl;
				phy->setRadioState(Radio::TX);
				changeDisplayColor(YELLOW);
				macState = SEND_PREAMBLE;
				scheduleAt(simTime() + slotDuration, stop_preambles);
				return;
			}
			// if not, go back to sleep and wake up after a full period
			else
			{
				debugEV << "State CCA, message CCA_TIMEOUT, new state SLEEP"
					   << endl;
				scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				phy->setRadioState(Radio::SLEEP);
				changeDisplayColor(BLACK);
				return;
			}
		}
		// during CCA, we received a preamble. Go to state WAIT_DATA and
		// schedule the timeout.
		if (msg->getKind() == BMAC_PREAMBLE)
		{
			nbRxPreambles++;
			debugEV << "State CCA, message BMAC_PREAMBLE received, new state"
					  " WAIT_DATA" << endl;
			macState = WAIT_DATA;
			cancelEvent(cca_timeout);
			scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
			delete msg;
			return;
		}
		// this case is very, very, very improbable, but let's do it.
		// if in CCA and the node receives directly the data packet, switch to
		// state WAIT_DATA and re-send the message
		if (msg->getKind() == BMAC_DATA)
		{
			nbRxDataPackets++;
			debugEV << "State CCA, message BMAC_DATA, new state WAIT_DATA"
				   << endl;
			macState = WAIT_DATA;
			cancelEvent(cca_timeout);
			scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
			scheduleAt(simTime(), msg);
			return;
		}
		//in case we get an ACK, we simply dicard it, because it means the end
		//of another communication
		if (msg->getKind() == BMAC_ACK)
		{
			debugEV << "State CCA, message BMAC_ACK, new state CCA" << endl;
			delete msg;
			return;
		}
		break;

	case SEND_PREAMBLE:
		if (msg->getKind() == BMAC_SEND_PREAMBLE)
		{
			debugEV << "State SEND_PREAMBLE, message BMAC_SEND_PREAMBLE, new"
					  " state SEND_PREAMBLE" << endl;
			sendPreamble();
			scheduleAt(simTime() + 0.5f*checkInterval, send_preamble);
			macState = SEND_PREAMBLE;
			return;
		}
		// simply change the state to SEND_DATA
		if (msg->getKind() == BMAC_STOP_PREAMBLES)
		{
			debugEV << "State SEND_PREAMBLE, message BMAC_STOP_PREAMBLES, new"
					  " state SEND_DATA" << endl;
			macState = SEND_DATA;
			txAttempts = 1;
			return;
		}
		break;

	case SEND_DATA:
		if ((msg->getKind() == BMAC_SEND_PREAMBLE)
			|| (msg->getKind() == BMAC_RESEND_DATA))
		{
			debugEV << "State SEND_DATA, message BMAC_SEND_PREAMBLE or"
					  " BMAC_RESEND_DATA, new state WAIT_TX_DATA_OVER" << endl;
			// send the data packet
			sendDataPacket();
			macState = WAIT_TX_DATA_OVER;
			return;
		}
		break;

	case WAIT_TX_DATA_OVER:
		if (msg->getKind() == BMAC_DATA_TX_OVER)
		{
			if ((useMacAcks) && !LAddress::isL2Broadcast( lastDataPktDestAddr ))
			{
				debugEV << "State WAIT_TX_DATA_OVER, message BMAC_DATA_TX_OVER,"
						  " new state WAIT_ACK" << endl;
				macState = WAIT_ACK;
				phy->setRadioState(Radio::RX);
				changeDisplayColor(GREEN);
				scheduleAt(simTime()+checkInterval, ack_timeout);
			}
			else
			{
				debugEV << "State WAIT_TX_DATA_OVER, message BMAC_DATA_TX_OVER,"
						  " new state  SLEEP" << endl;
				delete macQueue.front();
				macQueue.pop_front();
				// if something in the queue, wakeup soon.
				if (macQueue.size() > 0)
					scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
				else
					scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				phy->setRadioState(Radio::SLEEP);
				changeDisplayColor(BLACK);
			}
			return;
		}
		break;
	case WAIT_ACK:
		if (msg->getKind() == BMAC_ACK_TIMEOUT)
		{
			// No ACK received. try again or drop.
			if (txAttempts < maxTxAttempts)
			{
				debugEV << "State WAIT_ACK, message BMAC_ACK_TIMEOUT, new state"
						  " SEND_DATA" << endl;
				txAttempts++;
				macState = SEND_PREAMBLE;
				scheduleAt(simTime() + slotDuration, stop_preambles);
				phy->setRadioState(Radio::TX);
				changeDisplayColor(YELLOW);
			}
			else
			{
				debugEV << "State WAIT_ACK, message BMAC_ACK_TIMEOUT, new state"
						  " SLEEP" << endl;
				//drop the packet
				delete macQueue.front();
				macQueue.pop_front();
				// if something in the queue, wakeup soon.
				if (macQueue.size() > 0)
					scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
				else
					scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				phy->setRadioState(Radio::SLEEP);
				changeDisplayColor(BLACK);
				nbMissedAcks++;
			}
			return;
		}
		//ignore and other packets
		if ((msg->getKind() == BMAC_DATA) || (msg->getKind() == BMAC_PREAMBLE))
		{
			debugEV << "State WAIT_ACK, message BMAC_DATA or BMAC_PREMABLE, new"
					  " state WAIT_ACK" << endl;
			delete msg;
			return;
		}
		if (msg->getKind() == BMAC_ACK)
		{
			debugEV << "State WAIT_ACK, message BMAC_ACK" << endl;
			MacPkt*                 mac = static_cast<MacPkt *>(msg);
			const LAddress::L2Type& src = mac->getSrcAddr();
			// the right ACK is received..
			debugEV << "We are waiting for ACK from : " << lastDataPktDestAddr
				   << ", and ACK came from : " << src << endl;
			if (src == lastDataPktDestAddr)
			{
				debugEV << "New state SLEEP" << endl;
				nbRecvdAcks++;
				lastDataPktDestAddr = LAddress::L2BROADCAST;
				cancelEvent(ack_timeout);
				delete macQueue.front();
				macQueue.pop_front();
				// if something in the queue, wakeup soon.
				if (macQueue.size() > 0)
					scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
				else
					scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				phy->setRadioState(Radio::SLEEP);
				changeDisplayColor(BLACK);
				lastDataPktDestAddr = LAddress::L2BROADCAST;
			}
			delete msg;
			return;
		}
		break;
	case WAIT_DATA:
		if(msg->getKind() == BMAC_PREAMBLE)
		{
			//nothing happens
			debugEV << "State WAIT_DATA, message BMAC_PREAMBLE, new state"
					  " WAIT_DATA" << endl;
			nbRxPreambles++;
			delete msg;
			return;
		}
		if(msg->getKind() == BMAC_ACK)
		{
			//nothing happens
			debugEV << "State WAIT_DATA, message BMAC_ACK, new state WAIT_DATA"
				   << endl;
			delete msg;
			return;
		}
		if (msg->getKind() == BMAC_DATA)
		{
			nbRxDataPackets++;
			MacPkt*                 mac  = static_cast<MacPkt *>(msg);
			const LAddress::L2Type& dest = mac->getDestAddr();
			const LAddress::L2Type& src  = mac->getSrcAddr();
			if ((dest == myMacAddr) || LAddress::isL2Broadcast(dest)) {
				sendUp(decapsMsg(mac));
			} else {
				delete msg;
				msg = NULL;
				mac = NULL;
			}

			cancelEvent(data_timeout);
			if ((useMacAcks) && (dest == myMacAddr))
			{
				debugEV << "State WAIT_DATA, message BMAC_DATA, new state"
						  " SEND_ACK" << endl;
				macState = SEND_ACK;
				lastDataPktSrcAddr = src;
				phy->setRadioState(Radio::TX);
				changeDisplayColor(YELLOW);
			}
			else
			{
				debugEV << "State WAIT_DATA, message BMAC_DATA, new state SLEEP"
					   << endl;
				// if something in the queue, wakeup soon.
				if (macQueue.size() > 0)
					scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
				else
					scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				phy->setRadioState(Radio::SLEEP);
				changeDisplayColor(BLACK);
			}
			return;
		}
		if (msg->getKind() == BMAC_DATA_TIMEOUT)
		{
			debugEV << "State WAIT_DATA, message BMAC_DATA_TIMEOUT, new state"
					  " SLEEP" << endl;
			// if something in the queue, wakeup soon.
			if (macQueue.size() > 0)
				scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
			else
				scheduleAt(simTime() + slotDuration, wakeup);
			macState = SLEEP;
			phy->setRadioState(Radio::SLEEP);
			changeDisplayColor(BLACK);
			return;
		}
		break;
	case SEND_ACK:
		if (msg->getKind() == BMAC_SEND_ACK)
		{
			debugEV << "State SEND_ACK, message BMAC_SEND_ACK, new state"
					  " WAIT_ACK_TX" << endl;
			// send now the ack packet
			sendMacAck();
			macState = WAIT_ACK_TX;
			return;
		}
		break;
	case WAIT_ACK_TX:
		if (msg->getKind() == BMAC_ACK_TX_OVER)
		{
			debugEV << "State WAIT_ACK_TX, message BMAC_ACK_TX_OVER, new state"
					  " SLEEP" << endl;
			// ack sent, go to sleep now.
			// if something in the queue, wakeup soon.
			if (macQueue.size() > 0)
				scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
			else
				scheduleAt(simTime() + slotDuration, wakeup);
			macState = SLEEP;
			phy->setRadioState(Radio::SLEEP);
			changeDisplayColor(BLACK);
			lastDataPktSrcAddr = LAddress::L2BROADCAST;
			return;
		}
		break;
	}
	opp_error("Undefined event of type %d in state %d (Radio state %d)!",
			  msg->getKind(), macState, phy->getRadioState());
}


/**
 * Handle BMAC preambles and received data packets.
 */
void BMacLayer::handleLowerMsg(cMessage *msg)
{
	// simply pass the massage as self message, to be processed by the FSM.
	handleSelfMsg(msg);
}

void BMacLayer::sendDataPacket()
{
	nbTxDataPackets++;
	MacPkt *pkt = macQueue.front()->dup();
	attachSignal(pkt);
	lastDataPktDestAddr = pkt->getDestAddr();
	pkt->setKind(BMAC_DATA);
	sendDown(pkt);
}

/**
 * Handle transmission over messages: either send another preambles or the data
 * packet itself.
 */
void BMacLayer::handleLowerControl(cMessage *msg)
{
	// Transmission of one packet is over
    if(msg->getKind() == MacToPhyInterface::TX_OVER) {
    	if (macState == WAIT_TX_DATA_OVER)
    	{
    		scheduleAt(simTime(), data_tx_over);
    	}
    	if (macState == WAIT_ACK_TX)
    	{
    		scheduleAt(simTime(), ack_tx_over);
    	}
    }
    // Radio switching (to RX or TX) ir over, ignore switching to SLEEP.
    else if(msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
    	// we just switched to TX after CCA, so simply send the first
    	// sendPremable self message
    	if ((macState == SEND_PREAMBLE) && (phy->getRadioState() == Radio::TX))
    	{
    		scheduleAt(simTime(), send_preamble);
    	}
    	if ((macState == SEND_ACK) && (phy->getRadioState() == Radio::TX))
    	{
    		scheduleAt(simTime(), send_ack);
    	}
    	// we were waiting for acks, but none came. we switched to TX and now
    	// need to resend data
    	if ((macState == SEND_DATA) && (phy->getRadioState() == Radio::TX))
    	{
    		scheduleAt(simTime(), resend_data);
    	}

    }
    else {
        debugEV << "control message with wrong kind -- deleting\n";
    }
    delete msg;
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all
 * needed header fields.
 */
bool BMacLayer::addToQueue(cMessage *msg)
{
	if (macQueue.size() >= queueLength) {
		// queue is full, message has to be deleted
		debugEV << "New packet arrived, but queue is FULL, so new packet is"
				  " deleted\n";
		msg->setName("MAC ERROR");
		msg->setKind(PACKET_DROPPED);
		sendControlUp(msg);
		droppedPacket.setReason(DroppedPacket::QUEUE);
		emit(BaseLayer::catDroppedPacketSignal, &droppedPacket);
		nbDroppedDataPackets++;

		return false;
	}

	MacPkt *macPkt = new MacPkt(msg->getName());
	macPkt->setBitLength(headerLength);
	cObject *const cInfo = msg->removeControlInfo();
	//EV<<"CSMA received a message from upper layer, name is "
	//  << msg->getName() <<", CInfo removed, mac addr="
	//  << cInfo->getNextHopMac()<<endl;
	macPkt->setDestAddr(getUpperDestinationFromControlInfo(cInfo));
	delete cInfo;
	macPkt->setSrcAddr(myMacAddr);

	assert(static_cast<cPacket*>(msg));
	macPkt->encapsulate(static_cast<cPacket*>(msg));

	macQueue.push_back(macPkt);
	debugEV << "Max queue length: " << queueLength << ", packet put in queue"
			  "\n  queue size: " << macQueue.size() << " macState: "
			  << macState << endl;
	return true;
}

void BMacLayer::attachSignal(MacPkt *macPkt)
{
	//calc signal duration
	simtime_t duration = macPkt->getBitLength() / bitrate;
	//create and initialize control info with new signal
	setDownControlInfo(macPkt, createSignal(simTime(), duration, txPower, bitrate));
}

/**
 * Change the color of the node for animation purposes.
 */

void BMacLayer::changeDisplayColor(BMAC_COLORS color)
{
	if (!animation)
		return;
	cDisplayString& dispStr = findHost()->getDisplayString();
	//b=40,40,rect,black,black,2"
	if (color == GREEN)
		dispStr.setTagArg("b", 3, "green");
		//dispStr.parse("b=40,40,rect,green,green,2");
	if (color == BLUE)
		dispStr.setTagArg("b", 3, "blue");
				//dispStr.parse("b=40,40,rect,blue,blue,2");
	if (color == RED)
		dispStr.setTagArg("b", 3, "red");
				//dispStr.parse("b=40,40,rect,red,red,2");
	if (color == BLACK)
		dispStr.setTagArg("b", 3, "black");
				//dispStr.parse("b=40,40,rect,black,black,2");
	if (color == YELLOW)
		dispStr.setTagArg("b", 3, "yellow");
				//dispStr.parse("b=40,40,rect,yellow,yellow,2");
}

/*void BMacLayer::changeMacState(States newState)
{
	switch (macState)
	{
	case RX:
		timeRX += (simTime() - lastTime);
		break;
	case TX:
		timeTX += (simTime() - lastTime);
		break;
	case SLEEP:
		timeSleep += (simTime() - lastTime);
		break;
	case CCA:
		timeRX += (simTime() - lastTime);
	}
	lastTime = simTime();

	switch (newState)
	{
	case CCA:
		changeDisplayColor(GREEN);
		break;
	case TX:
		changeDisplayColor(BLUE);
		break;
	case SLEEP:
		changeDisplayColor(BLACK);
		break;
	case RX:
		changeDisplayColor(YELLOW);
		break;
	}

	macState = newState;
}*/
