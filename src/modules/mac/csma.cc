/* -*- mode:c++ -*- ********************************************************
 * file:        csma.cc
 *
 * author:      Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 * 				Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 *
 * copyright:   (C) 2007-2009 CSEM SA
 * 				(C) 2009 T.U. Eindhoven
 *				(C) 2004,2005,2006
 *              Telecommunication Networks Group (TKN) at Technische
 *              Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/
#include "csma.h"

#include <cassert>

#include "FWMath.h"
#include "BaseDecider.h"
#include "BaseArp.h"
#include "BasePhyLayer.h"
#include "BaseConnectionManager.h"
#include "FindModule.h"
#include "MacPkt_m.h"

Define_Module(csma);

/**
 * Initialize the of the omnetpp.ini variables in stage 1. In stage
 * two subscribe to the RadioState.
 */

void csma::initialize(int stage) {
	BaseMacLayer::initialize(stage);

	if (stage == 0) {

		useMACAcks = par("useMACAcks").boolValue();
		queueLength = par("queueLength");
		sifs = par("sifs");
		transmissionAttemptInterruptedByRx = false;
		nbTxFrames = 0;
		nbRxFrames = 0;
		nbMissedAcks = 0;
		nbTxAcks = 0;
		nbRecvdAcks = 0;
		nbDroppedFrames = 0;
		nbDuplicates = 0;
		nbBackoffs = 0;
		backoffValues = 0;
		stats = par("stats");
		trace = par("trace");
		macMaxCSMABackoffs = par("macMaxCSMABackoffs");
		macMaxFrameRetries = par("macMaxFrameRetries");
		macAckWaitDuration = par("macAckWaitDuration").doubleValue();
		aUnitBackoffPeriod = par("aUnitBackoffPeriod");
		ccaDetectionTime = par("ccaDetectionTime").doubleValue();
		rxSetupTime = par("rxSetupTime").doubleValue();
		aTurnaroundTime = par("aTurnaroundTime").doubleValue();
		bitrate = par("bitrate");
		ackLength = par("ackLength");
		ackMessage = NULL;

		//init parameters for backoff method
		std::string backoffMethodStr = par("backoffMethod").stdstringValue();
		if(backoffMethodStr == "exponential") {
			backoffMethod = EXPONENTIAL;
			macMinBE = par("macMinBE");
			macMaxBE = par("macMaxBE");
		}
		else {
			if(backoffMethodStr == "linear") {
				backoffMethod = LINEAR;
			}
			else if (backoffMethodStr == "constant") {
				backoffMethod = CONSTANT;
			}
			else {
				error("Unknown backoff method \"%s\".\
					   Use \"constant\", \"linear\" or \"\
					   \"exponential\".", backoffMethodStr.c_str());
			}
			initialCW = par("contentionWindow");
		}
		NB = 0;

		txPower = par("txPower").doubleValue();

		droppedPacket.setReason(DroppedPacket::NONE);
		nicId = getParentModule()->getId();

		// initialize the timers
		backoffTimer = new cMessage("timer-backoff");
		ccaTimer = new cMessage("timer-cca");
		sifsTimer = new cMessage("timer-sifs");
		rxAckTimer = new cMessage("timer-rxAck");
		macState = IDLE_1;
		txAttempts = 0;

		//check parameters for consistency
		cModule* phyModule = FindModule<BasePhyLayer*>
								::findSubModule(getParentModule());

		//aTurnaroundTime should match (be equal or bigger) the RX to TX
		//switching time of the radio
		if(phyModule->hasPar("timeRXToTX")) {
			simtime_t rxToTx = phyModule->par("timeRXToTX").doubleValue();
			if( rxToTx > aTurnaroundTime)
			{
				opp_warning("Parameter \"aTurnaroundTime\" (%f) does not match"
							" the radios RX to TX switching time (%f)! It"
							" should be equal or bigger",
							SIMTIME_DBL(aTurnaroundTime), SIMTIME_DBL(rxToTx));
			}
		}

	} else if(stage == 1) {
		BaseConnectionManager* cc = getConnectionManager();

    	if(cc->hasPar("pMax") && txPower > cc->par("pMax").doubleValue())
            opp_error("TranmitterPower can't be bigger than pMax in ConnectionManager! "
            		  "Please adjust your omnetpp.ini file accordingly");

    	debugEV << "queueLength = " << queueLength
		<< " bitrate = " << bitrate
		<< " backoff method = " << par("backoffMethod").stringValue() << endl;

		debugEV << "finished csma init stage 1." << endl;
	}
}

void csma::finish() {
	if (stats) {
		recordScalar("nbTxFrames", nbTxFrames);
		recordScalar("nbRxFrames", nbRxFrames);
		recordScalar("nbDroppedFrames", nbDroppedFrames);
		recordScalar("nbMissedAcks", nbMissedAcks);
		recordScalar("nbRecvdAcks", nbRecvdAcks);
		recordScalar("nbTxAcks", nbTxAcks);
		recordScalar("nbDuplicates", nbDuplicates);
		if (nbBackoffs > 0) {
			recordScalar("meanBackoff", backoffValues / nbBackoffs);
		} else {
			recordScalar("meanBackoff", 0);
		}
		recordScalar("nbBackoffs", nbBackoffs);
		recordScalar("backoffDurations", backoffValues);
	}
}

csma::~csma() {
	cancelAndDelete(backoffTimer);
	cancelAndDelete(ccaTimer);
	cancelAndDelete(sifsTimer);
	cancelAndDelete(rxAckTimer);
	if (ackMessage)
		delete ackMessage;
	MacQueue::iterator it;
	for (it = macQueue.begin(); it != macQueue.end(); ++it) {
		delete (*it);
	}
}

/**
 * Encapsulates the message to be transmitted and pass it on
 * to the FSM main method for further processing.
 */
void csma::handleUpperMsg(cMessage *msg) {
	//MacPkt *macPkt = encapsMsg(msg);
	MacPkt *macPkt = new MacPkt(msg->getName());
	macPkt->setBitLength(headerLength);
	cObject *const cInfo = msg->removeControlInfo();
	debugEV<<"CSMA received a message from upper layer, name is " << msg->getName() <<", CInfo removed, mac addr="<< getUpperDestinationFromControlInfo(cInfo) << endl;
	LAddress::L2Type dest = getUpperDestinationFromControlInfo(cInfo);
	macPkt->setDestAddr(dest);
	delete cInfo;
	macPkt->setSrcAddr(myMacAddr);

	if(useMACAcks) {
		if(SeqNrParent.find(dest) == SeqNrParent.end()) {
			//no record of current parent -> add next sequence number to map
			SeqNrParent[dest] = 1;
			macPkt->setSequenceId(0);
			debugEV << "Adding a new parent to the map of Sequence numbers:" << dest << endl;
		}
		else {
			macPkt->setSequenceId(SeqNrParent[dest]);
			debugEV << "Packet send with sequence number = " << SeqNrParent[dest] << endl;
			SeqNrParent[dest]++;
		}
	}

	//RadioAccNoise3PhyControlInfo *pco = new RadioAccNoise3PhyControlInfo(bitrate);
	//macPkt->setControlInfo(pco);
	assert(static_cast<cPacket*>(msg));
	macPkt->encapsulate(static_cast<cPacket*>(msg));
	debugEV <<"pkt encapsulated, length: " << macPkt->getBitLength() << "\n";
	executeMac(EV_SEND_REQUEST, macPkt);
}

void csma::updateStatusIdle(t_mac_event event, cMessage *msg) {
	switch (event) {
	case EV_SEND_REQUEST:
		if (macQueue.size() <= queueLength) {
			macQueue.push_back(static_cast<MacPkt *> (msg));
			debugEV<<"(1) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff avail]: startTimerBackOff -> BACKOFF." << endl;
			updateMacState(BACKOFF_2);
			NB = 0;
			//BE = macMinBE;
			startTimer(TIMER_BACKOFF);
		} else {
			// queue is full, message has to be deleted
			debugEV << "(12) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff not avail]: dropping packet -> IDLE." << endl;
			msg->setName("MAC ERROR");
			msg->setKind(PACKET_DROPPED);
			sendControlUp(msg);
			droppedPacket.setReason(DroppedPacket::QUEUE);
			emit(BaseLayer::catDroppedPacketSignal, &droppedPacket);
			updateMacState(IDLE_1);
		}
		break;
	case EV_DUPLICATE_RECEIVED:
		debugEV << "(15) FSM State IDLE_1, EV_DUPLICATE_RECEIVED: setting up radio tx -> WAITSIFS." << endl;
		//sendUp(decapsMsg(static_cast<MacSeqPkt *>(msg)));
		delete msg;

		if(useMACAcks) {
			phy->setRadioState(Radio::TX);
			updateMacState(WAITSIFS_6);
			startTimer(TIMER_SIFS);
		}
		break;

	case EV_FRAME_RECEIVED:
		debugEV << "(15) FSM State IDLE_1, EV_FRAME_RECEIVED: setting up radio tx -> WAITSIFS." << endl;
		sendUp(decapsMsg(static_cast<MacPkt *>(msg)));
		nbRxFrames++;
		delete msg;

		if(useMACAcks) {
			phy->setRadioState(Radio::TX);
			updateMacState(WAITSIFS_6);
			startTimer(TIMER_SIFS);
		}
		break;

	case EV_BROADCAST_RECEIVED:
		debugEV << "(23) FSM State IDLE_1, EV_BROADCAST_RECEIVED: Nothing to do." << endl;
		nbRxFrames++;
		sendUp(decapsMsg(static_cast<MacPkt *>(msg)));
		delete msg;
		break;
	default:
		fsmError(event, msg);
		break;
	}
}

void csma::updateStatusBackoff(t_mac_event event, cMessage *msg) {
	switch (event) {
	case EV_TIMER_BACKOFF:
		debugEV<< "(2) FSM State BACKOFF, EV_TIMER_BACKOFF:"
		<< " starting CCA timer." << endl;
		startTimer(TIMER_CCA);
		updateMacState(CCA_3);
		phy->setRadioState(Radio::RX);
		break;
	case EV_DUPLICATE_RECEIVED:
		// suspend current transmission attempt,
		// transmit ack,
		// and resume transmission when entering manageQueue()
		debugEV << "(28) FSM State BACKOFF, EV_DUPLICATE_RECEIVED:";
		if(useMACAcks) {
			debugEV << "suspending current transmit tentative and transmitting ack";
			transmissionAttemptInterruptedByRx = true;
			cancelEvent(backoffTimer);
			phy->setRadioState(Radio::TX);
			updateMacState(WAITSIFS_6);
			startTimer(TIMER_SIFS);
		} else {
			debugEV << "Nothing to do.";
		}
		//sendUp(decapsMsg(static_cast<MacSeqPkt *>(msg)));
		delete msg;

		break;
	case EV_FRAME_RECEIVED:
		// suspend current transmission attempt,
		// transmit ack,
		// and resume transmission when entering manageQueue()
		debugEV << "(28) FSM State BACKOFF, EV_FRAME_RECEIVED:";
		if(useMACAcks) {
			debugEV << "suspending current transmit tentative and transmitting ack";
			transmissionAttemptInterruptedByRx = true;
			cancelEvent(backoffTimer);

			phy->setRadioState(Radio::TX);
			updateMacState(WAITSIFS_6);
			startTimer(TIMER_SIFS);
		} else {
			debugEV << "sending frame up and resuming normal operation.";
		}
		sendUp(decapsMsg(static_cast<MacPkt *>(msg)));
		delete msg;
		break;
	case EV_BROADCAST_RECEIVED:
		debugEV << "(29) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
		<< "sending frame up and resuming normal operation." <<endl;
		sendUp(decapsMsg(static_cast<MacPkt *>(msg)));
		delete msg;
		break;
	default:
		fsmError(event, msg);
		break;
	}
}

void csma::attachSignal(MacPkt* mac, simtime_t_cref startTime) {
	simtime_t duration = (mac->getBitLength() + phyHeaderLength)/bitrate;
	setDownControlInfo(mac, createSignal(startTime, duration, txPower, bitrate));
}

void csma::updateStatusCCA(t_mac_event event, cMessage *msg) {
	switch (event) {
	case EV_TIMER_CCA:
	{
		debugEV<< "(25) FSM State CCA_3, EV_TIMER_CCA" << endl;
		bool isIdle = phy->getChannelState().isIdle();
		if(isIdle) {
			debugEV << "(3) FSM State CCA_3, EV_TIMER_CCA, [Channel Idle]: -> TRANSMITFRAME_4." << endl;
			updateMacState(TRANSMITFRAME_4);
			phy->setRadioState(Radio::TX);
			MacPkt * mac = check_and_cast<MacPkt *>(macQueue.front()->dup());
			attachSignal(mac, simTime()+aTurnaroundTime);
			//sendDown(msg);
			// give time for the radio to be in Tx state before transmitting
			sendDelayed(mac, aTurnaroundTime, lowerLayerOut);
			nbTxFrames++;
		} else {
			// Channel was busy, increment 802.15.4 backoff timers as specified.
			debugEV << "(7) FSM State CCA_3, EV_TIMER_CCA, [Channel Busy]: "
			<< " increment counters." << endl;
			NB = NB+1;
			//BE = std::min(BE+1, macMaxBE);

			// decide if we go for another backoff or if we drop the frame.
			if(NB> macMaxCSMABackoffs) {
				// drop the frame
				debugEV << "Tried " << NB << " backoffs, all reported a busy "
				<< "channel. Dropping the packet." << endl;
				cMessage * mac = macQueue.front();
				macQueue.pop_front();
				txAttempts = 0;
				nbDroppedFrames++;
				mac->setName("MAC ERROR");
				mac->setKind(PACKET_DROPPED);
				sendControlUp(mac);
				manageQueue();
			} else {
				// redo backoff
				updateMacState(BACKOFF_2);
				startTimer(TIMER_BACKOFF);
			}
		}
		break;
	}
	case EV_DUPLICATE_RECEIVED:
		debugEV << "(26) FSM State CCA_3, EV_DUPLICATE_RECEIVED:";
		if(useMACAcks) {
			debugEV << " setting up radio tx -> WAITSIFS." << endl;
			// suspend current transmission attempt,
			// transmit ack,
			// and resume transmission when entering manageQueue()
			transmissionAttemptInterruptedByRx = true;
			cancelEvent(ccaTimer);

			phy->setRadioState(Radio::TX);
			updateMacState(WAITSIFS_6);
			startTimer(TIMER_SIFS);
		} else {
			debugEV << " Nothing to do." << endl;
		}
		//sendUp(decapsMsg(static_cast<MacPkt *>(msg)));
		delete msg;
		break;

	case EV_FRAME_RECEIVED:
		debugEV << "(26) FSM State CCA_3, EV_FRAME_RECEIVED:";
		if(useMACAcks) {
			debugEV << " setting up radio tx -> WAITSIFS." << endl;
			// suspend current transmission attempt,
			// transmit ack,
			// and resume transmission when entering manageQueue()
			transmissionAttemptInterruptedByRx = true;
			cancelEvent(ccaTimer);
			phy->setRadioState(Radio::TX);
			updateMacState(WAITSIFS_6);
			startTimer(TIMER_SIFS);
		} else {
			debugEV << " Nothing to do." << endl;
		}
		sendUp(decapsMsg(static_cast<MacPkt *>(msg)));
		delete msg;
		break;
	case EV_BROADCAST_RECEIVED:
		debugEV << "(24) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
		<< " Nothing to do." << endl;
		sendUp(decapsMsg(static_cast<MacPkt *>(msg)));
		delete msg;
		break;
	default:
		fsmError(event, msg);
		break;
	}
}

void csma::updateStatusTransmitFrame(t_mac_event event, cMessage *msg) {
	if (event == EV_FRAME_TRANSMITTED) {
		//    delete msg;
		MacPkt * packet = macQueue.front();
		phy->setRadioState(Radio::RX);

		bool expectAck = useMACAcks;
		if (!LAddress::isL2Broadcast(packet->getDestAddr())) {
			//unicast
			debugEV << "(4) FSM State TRANSMITFRAME_4, "
			   << "EV_FRAME_TRANSMITTED [Unicast]: ";
		} else {
			//broadcast
			debugEV << "(27) FSM State TRANSMITFRAME_4, EV_FRAME_TRANSMITTED "
			   << " [Broadcast]";
			expectAck = false;
		}

		if(expectAck) {
			debugEV << "RadioSetupRx -> WAITACK." << endl;
			updateMacState(WAITACK_5);
			startTimer(TIMER_RX_ACK);
		} else {
			debugEV << ": RadioSetupRx, manageQueue..." << endl;
			macQueue.pop_front();
			delete packet;
			manageQueue();
		}
	} else {
		fsmError(event, msg);
	}
}

void csma::updateStatusWaitAck(t_mac_event event, cMessage *msg) {
	assert(useMACAcks);

	cMessage * mac;
	switch (event) {
	case EV_ACK_RECEIVED:
		debugEV<< "(5) FSM State WAITACK_5, EV_ACK_RECEIVED: "
		<< " ProcessAck, manageQueue..." << endl;
		if(rxAckTimer->isScheduled())
		cancelEvent(rxAckTimer);
		mac = static_cast<cMessage *>(macQueue.front());
		macQueue.pop_front();
		txAttempts = 0;
		mac->setName("MAC SUCCESS");
		mac->setKind(TX_OVER);
		sendControlUp(mac);
		delete msg;
		manageQueue();
		break;
	case EV_ACK_TIMEOUT:
		debugEV << "(12) FSM State WAITACK_5, EV_ACK_TIMEOUT:"
		<< " incrementCounter/dropPacket, manageQueue..." << endl;
		manageMissingAck(event, msg);
		break;
	case EV_BROADCAST_RECEIVED:
	case EV_FRAME_RECEIVED:
		sendUp(decapsMsg(static_cast<MacPkt*>(msg)));
		break;
	case EV_DUPLICATE_RECEIVED:
		debugEV << "Error ! Received a frame during SIFS !" << endl;
		delete msg;
		break;
	default:
		fsmError(event, msg);
		break;
	}

}

void csma::manageMissingAck(t_mac_event event, cMessage *msg) {
	if (txAttempts < macMaxFrameRetries + 1) {
		// increment counter
		txAttempts++;
		debugEV<< "I will retransmit this packet (I already tried "
		<< txAttempts << " times)." << endl;
	} else {
		// drop packet
		debugEV << "Packet was transmitted " << txAttempts
		<< " times and I never got an Ack. I drop the packet." << endl;
		cMessage * mac = macQueue.front();
		macQueue.pop_front();
		txAttempts = 0;
		mac->setName("MAC ERROR");
		mac->setKind(PACKET_DROPPED);
		sendControlUp(mac);
	}
	manageQueue();
}
void csma::updateStatusSIFS(t_mac_event event, cMessage *msg) {
	assert(useMACAcks);

	switch (event) {
	case EV_TIMER_SIFS:
		debugEV<< "(17) FSM State WAITSIFS_6, EV_TIMER_SIFS:"
		<< " sendAck -> TRANSMITACK." << endl;
		updateMacState(TRANSMITACK_7);
		attachSignal(ackMessage, simTime());
		sendDown(ackMessage);
		nbTxAcks++;
		//		sendDelayed(ackMessage, aTurnaroundTime, lowerLayerOut);
		ackMessage = NULL;
		break;
	case EV_TIMER_BACKOFF:
		// Backoff timer has expired while receiving a frame. Restart it
		// and stay here.
		debugEV << "(16) FSM State WAITSIFS_6, EV_TIMER_BACKOFF. "
		<< "Restart backoff timer and don't move." << endl;
		startTimer(TIMER_BACKOFF);
		break;
	case EV_BROADCAST_RECEIVED:
	case EV_FRAME_RECEIVED:
		EV << "Error ! Received a frame during SIFS !" << endl;
		sendUp(decapsMsg(static_cast<MacPkt*>(msg)));
		delete msg;
		break;
	default:
		fsmError(event, msg);
		break;
	}
}

void csma::updateStatusTransmitAck(t_mac_event event, cMessage *msg) {
	assert(useMACAcks);

	if (event == EV_FRAME_TRANSMITTED) {
		debugEV<< "(19) FSM State TRANSMITACK_7, EV_FRAME_TRANSMITTED:"
		<< " ->manageQueue." << endl;
		phy->setRadioState(Radio::RX);
		//		delete msg;
		manageQueue();
	} else {
		fsmError(event, msg);
	}
}

void csma::updateStatusNotIdle(cMessage *msg) {
	debugEV<< "(20) FSM State NOT IDLE, EV_SEND_REQUEST. Is a TxBuffer available ?" << endl;
	if (macQueue.size() <= queueLength) {
		macQueue.push_back(static_cast<MacPkt *>(msg));
		debugEV << "(21) FSM State NOT IDLE, EV_SEND_REQUEST"
		<<" and [TxBuff avail]: enqueue packet and don't move." << endl;
	} else {
		// queue is full, message has to be deleted
		debugEV << "(22) FSM State NOT IDLE, EV_SEND_REQUEST"
		<< " and [TxBuff not avail]: dropping packet and don't move."
		<< endl;
		msg->setName("MAC ERROR");
		msg->setKind(PACKET_DROPPED);
		sendControlUp(msg);
		droppedPacket.setReason(DroppedPacket::QUEUE);
		emit(BaseLayer::catDroppedPacketSignal, &droppedPacket);
	}

}
/**
 * Updates state machine.
 */
void csma::executeMac(t_mac_event event, cMessage *msg) {
	debugEV<< "In executeMac" << endl;
	if(macState != IDLE_1 && event == EV_SEND_REQUEST) {
		updateStatusNotIdle(msg);
	} else {
		switch(macState) {
		case IDLE_1:
			updateStatusIdle(event, msg);
			break;
		case BACKOFF_2:
			updateStatusBackoff(event, msg);
			break;
		case CCA_3:
			updateStatusCCA(event, msg);
			break;
		case TRANSMITFRAME_4:
			updateStatusTransmitFrame(event, msg);
			break;
		case WAITACK_5:
			updateStatusWaitAck(event, msg);
			break;
		case WAITSIFS_6:
			updateStatusSIFS(event, msg);
			break;
		case TRANSMITACK_7:
			updateStatusTransmitAck(event, msg);
			break;
		default:
			EV << "Error in CSMA FSM: an unknown state has been reached. macState=" << macState << endl;
			break;
		}
	}
}

void csma::manageQueue() {
	if (macQueue.size() != 0) {
		debugEV<< "(manageQueue) there are " << macQueue.size() << " packets to send, entering backoff wait state." << endl;
		if( transmissionAttemptInterruptedByRx) {
			// resume a transmission cycle which was interrupted by
			// a frame reception during CCA check
			transmissionAttemptInterruptedByRx = false;
		} else {
			// initialize counters if we start a new transmission
			// cycle from zero
			NB = 0;
			//BE = macMinBE;
		}
		if(! backoffTimer->isScheduled()) {
		  startTimer(TIMER_BACKOFF);
		}
		updateMacState(BACKOFF_2);
	} else {
		debugEV << "(manageQueue) no packets to send, entering IDLE state." << endl;
		updateMacState(IDLE_1);
	}
}

void csma::updateMacState(t_mac_states newMacState) {
	macState = newMacState;
}

/*
 * Called by the FSM machine when an unknown transition is requested.
 */
void csma::fsmError(t_mac_event event, cMessage *msg) {
	EV<< "FSM Error ! In state " << macState << ", received unknown event:" << event << "." << endl;
	if (msg != NULL)
	delete msg;
}

void csma::startTimer(t_mac_timer timer) {
	if (timer == TIMER_BACKOFF) {
		scheduleAt(scheduleBackoff(), backoffTimer);
	} else if (timer == TIMER_CCA) {
		simtime_t ccaTime = rxSetupTime + ccaDetectionTime;
		debugEV<< "(startTimer) ccaTimer value=" << ccaTime
		<< "(rxSetupTime,ccaDetectionTime:" << rxSetupTime
		<< "," << ccaDetectionTime <<")." << endl;
		scheduleAt(simTime()+rxSetupTime+ccaDetectionTime, ccaTimer);
	} else if (timer==TIMER_SIFS) {
		assert(useMACAcks);
		debugEV << "(startTimer) sifsTimer value=" << sifs << endl;
		scheduleAt(simTime()+sifs, sifsTimer);
	} else if (timer==TIMER_RX_ACK) {
		assert(useMACAcks);
		debugEV << "(startTimer) rxAckTimer value=" << macAckWaitDuration << endl;
		scheduleAt(simTime()+macAckWaitDuration, rxAckTimer);
	} else {
		EV << "Unknown timer requested to start:" << timer << endl;
	}
}

simtime_t csma::scheduleBackoff() {

	simtime_t backoffTime;

	switch(backoffMethod) {
	case EXPONENTIAL:
	{
		int BE = std::min(macMinBE + NB, macMaxBE);
		int v = (1 << BE) - 1;
		int r = intuniform(0, v, 0);
		backoffTime = r * aUnitBackoffPeriod;

		debugEV<< "(startTimer) backoffTimer value=" << backoffTime
		<< " (BE=" << BE << ", 2^BE-1= " << v << "r="
		<< r << ")" << endl;
		break;
	}
	case LINEAR:
	{
		int slots = intuniform(1, initialCW + NB, 0);
		backoffTime = slots * aUnitBackoffPeriod;
		debugEV<< "(startTimer) backoffTimer value=" << backoffTime << endl;
		break;
	}
	case CONSTANT:
	{
		int slots = intuniform(1, initialCW, 0);
		backoffTime = slots * aUnitBackoffPeriod;
		debugEV<< "(startTimer) backoffTimer value=" << backoffTime << endl;
		break;
	}
	default:
		error("Unknown backoff method!");
		break;
	}

	nbBackoffs = nbBackoffs + 1;
	backoffValues = backoffValues + SIMTIME_DBL(backoffTime);

	return backoffTime + simTime();
}

/*
 * Binds timers to events and executes FSM.
 */
void csma::handleSelfMsg(cMessage *msg) {
	debugEV<< "timer routine." << endl;
	if(msg==backoffTimer)
		executeMac(EV_TIMER_BACKOFF, msg);
	else if(msg==ccaTimer)
		executeMac(EV_TIMER_CCA, msg);
	else if(msg==sifsTimer)
		executeMac(EV_TIMER_SIFS, msg);
	else if(msg==rxAckTimer) {
		nbMissedAcks++;
		executeMac(EV_ACK_TIMEOUT, msg);
	} else
		EV << "CSMA Error: unknown timer fired:" << msg << endl;
}

/**
 * Compares the address of this Host with the destination address in
 * frame. Generates the corresponding event.
 */
void csma::handleLowerMsg(cMessage *msg) {
	MacPkt*                 macPkt     = static_cast<MacPkt *> (msg);
	const LAddress::L2Type& src        = macPkt->getSrcAddr();
	const LAddress::L2Type& dest       = macPkt->getDestAddr();
	long                    ExpectedNr = 0;

	debugEV<< "Received frame name= " << macPkt->getName()
	<< ", myState=" << macState << " src=" << macPkt->getSrcAddr()
	<< " dst=" << macPkt->getDestAddr() << " myAddr="
	<< myMacAddr << endl;

	if(macPkt->getDestAddr() == myMacAddr)
	{
		if(!useMACAcks) {
			debugEV << "Received a data packet addressed to me." << endl;
//			nbRxFrames++;
			executeMac(EV_FRAME_RECEIVED, macPkt);
		}
		else {
			long SeqNr = macPkt->getSequenceId();

			if(strcmp(macPkt->getName(), "CSMA-Ack") != 0) {
				// This is a data message addressed to us
				// and we should send an ack.
				// we build the ack packet here because we need to
				// copy data from macPkt (src).
				debugEV << "Received a data packet addressed to me,"
				   << " preparing an ack..." << endl;

//				nbRxFrames++;

				if(ackMessage != NULL)
					delete ackMessage;
				ackMessage = new MacPkt("CSMA-Ack");
				ackMessage->setSrcAddr(myMacAddr);
				ackMessage->setDestAddr(macPkt->getSrcAddr());
				ackMessage->setBitLength(ackLength);
				//Check for duplicates by checking expected seqNr of sender
				if(SeqNrChild.find(src) == SeqNrChild.end()) {
					//no record of current child -> add expected next number to map
					SeqNrChild[src] = SeqNr + 1;
					debugEV << "Adding a new child to the map of Sequence numbers:" << src << endl;
					executeMac(EV_FRAME_RECEIVED, macPkt);
				}
				else {
					ExpectedNr = SeqNrChild[src];
					debugEV << "Expected Sequence number is " << ExpectedNr <<
					" and number of packet is " << SeqNr << endl;
					if(SeqNr < ExpectedNr) {
						//Duplicate Packet, count and do not send to upper layer
						nbDuplicates++;
						executeMac(EV_DUPLICATE_RECEIVED, macPkt);
					}
					else {
						SeqNrChild[src] = SeqNr + 1;
						executeMac(EV_FRAME_RECEIVED, macPkt);
					}
				}

			} else if(macQueue.size() != 0) {

				// message is an ack, and it is for us.
				// Is it from the right node ?
				MacPkt * firstPacket = static_cast<MacPkt *>(macQueue.front());
				if(macPkt->getSrcAddr() == firstPacket->getDestAddr()) {
					nbRecvdAcks++;
					executeMac(EV_ACK_RECEIVED, macPkt);
				} else {
					EV << "Error! Received an ack from an unexpected source: src=" << macPkt->getSrcAddr() << ", I was expecting from node addr=" << firstPacket->getDestAddr() << endl;
					delete macPkt;
				}
			} else {
				EV << "Error! Received an Ack while my send queue was empty. src=" << macPkt->getSrcAddr() << "." << endl;
				delete macPkt;
			}
		}
	}
	else if (LAddress::isL2Broadcast(dest)) {
		executeMac(EV_BROADCAST_RECEIVED, macPkt);
	} else {
		debugEV << "packet not for me, deleting...\n";
		delete macPkt;
	}
}

void csma::handleLowerControl(cMessage *msg) {
	if (msg->getKind() == MacToPhyInterface::TX_OVER) {
		executeMac(EV_FRAME_TRANSMITTED, msg);
	} else if (msg->getKind() == BaseDecider::PACKET_DROPPED) {
		debugEV<< "control message: PACKED DROPPED" << endl;
	} else if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
		debugEV<< "control message: RADIO_SWITCHING_OVER" << endl;
	} else {
		EV << "Invalid control message type (type=NOTHING) : name="
		<< msg->getName() << " modulesrc="
		<< msg->getSenderModule()->getFullPath()
		<< "." << endl;
	}
	delete msg;
}

cPacket *csma::decapsMsg(MacPkt * macPkt) {
	cPacket * msg = macPkt->decapsulate();
	setUpControlInfo(msg, macPkt->getSrcAddr());

	return msg;
}

