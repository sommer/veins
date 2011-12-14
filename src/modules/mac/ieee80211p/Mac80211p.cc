//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "Mac80211p.h"

Define_Module(Mac80211p);

/**
 * Initialize the of the omnetpp.ini variables in stage 1. In stage
 * two subscribe to the RadioState.
 */

void Mac80211p::initialize(int stage) {
	BaseMacLayer::initialize(stage);

	if (stage == 0) {

		_1609_Mac = FindModule<Mac80211pToMac1609_4Interface*>::findSubModule(getParentModule());
		assert(_1609_Mac);

		packetToSend = NULL;

		macMaxCSMABackoffs = par("macMaxCSMABackoffs");
		macMaxFrameRetries = par("macMaxFrameRetries");
		txPower = par("txPower").doubleValue();

		bitrate = par("bitrate");
		checkBitrate(bitrate);

		//init parameters for backoff method
		currentNumBackoffs = 0;

		droppedPacket.setReason(DroppedPacket::NONE);

		//addresses
		myNicId = getParentModule()->getId();
		myMacAddress = myNicId;

		// initialize the timers
		backoffTimer = new cMessage("timer-backoff");
		ccaTimer = new cMessage("timer-cca");

		//initial statemachine and status values
		macState = IDLE_1;
		txAttempts = 0;
		currentCW = 0;

		//init statistic
		statsReceivedPackets = 0;
		statsReceivedBroadcasts = 0;
		statsSentPackets = 0;
		statsLostPackets = 0;
		statsNumBackoffs = 0;
		statsDroppedPackets = 0;
		statsNumTooLittleTime = 0;
		statsTotalBackoffDuration = 0;

		/*
		 * Taken from Table 7-73a of IEEE P802.11pTM/D10.0 Draft Standard for Information Technology —
		 */
		SCHcontentionParamaters.push_back(new ContentionParameters(7,CWMIN_11P,CWMAX_11P)); //Priority 0 AC_BK
		SCHcontentionParamaters.push_back(new ContentionParameters(3,CWMIN_11P,CWMAX_11P)); //Priority 1 AC_BE
		SCHcontentionParamaters.push_back(new ContentionParameters(2,(((CWMIN_11P+1)/2)-1),CWMAX_11P)); //Priority 2 AC_VI
		SCHcontentionParamaters.push_back(new ContentionParameters(2,(((CWMIN_11P+1)/4)-1),(((CWMAX_11P+1)/2)-1))); //Priority 3 AC_VO

		CCHcontentionParamaters.push_back(new ContentionParameters(9,CWMIN_11P,CWMAX_11P)); //Priority 0 AC_BK
		CCHcontentionParamaters.push_back(new ContentionParameters(6,(((CWMIN_11P+1)/2)-1),CWMAX_11P)); //Priority 1 AC_BE
		CCHcontentionParamaters.push_back(new ContentionParameters(3,(((CWMIN_11P+1)/4)-1),(((CWMIN_11P +1)/2)-1))); //Priority 2 AC_VI
		CCHcontentionParamaters.push_back(new ContentionParameters(2,(((CWMIN_11P+1)/4)-1),(((CWMIN_11P +1)/2)-1))); //Priority 3 AC_VO

	}
	else if (stage == 1) {
		BaseConnectionManager* cc = FindModule<BaseConnectionManager*>::findGlobalModule();

		if (txPower > cc->par("pMax").doubleValue())
			opp_error("TransmitterPower can't be bigger than pMax in ConnectionManager! "
			          "Please adjust your omnetpp.ini file accordingly.");
	}
}

void Mac80211p::finish() {
	for (uint32 i = 0; i < CCHcontentionParamaters.size(); i++) {
		delete CCHcontentionParamaters[i];
		delete SCHcontentionParamaters[i];
	}
	CCHcontentionParamaters.clear();
	SCHcontentionParamaters.clear();

	cancelAndDelete(backoffTimer);
	cancelAndDelete(ccaTimer);
	if (packetToSend != NULL) delete packetToSend;

	recordScalar("statsReceivedPackets",statsReceivedPackets);
	recordScalar("statsReceivedBroadcasts",statsReceivedBroadcasts);
	recordScalar("statsSentPackets",statsSentPackets);
	recordScalar("statsLostPackets",statsLostPackets);
	recordScalar("statsNumBackoffs",statsNumBackoffs);
	recordScalar("statsDroppedPackets",statsDroppedPackets);
	recordScalar("statsNumTooLittleTime",statsNumTooLittleTime);
	recordScalar("statsTotalBackoffDuration",statsTotalBackoffDuration);
}

/**
 * Encapsulates the message to be transmitted and pass it on
 * to the FSM main method for further processing.
 */
void Mac80211p::handleUpperMsg(cMessage* msg) {

	Mac80211Pkt* macPkt = dynamic_cast<Mac80211Pkt*>(msg);

	assert(dynamic_cast<Mac1609_4To80211pControlInfo*>(msg->getControlInfo()));

	//currently only broadcasts are supported
	macPkt->setDestAddr(LAddress::L2BROADCAST);
	macPkt->setSrcAddr(myMacAddress);

	executeMac(EV_SEND_REQUEST, macPkt);
}

void Mac80211p::updateStatusIdle(t_mac_event event, cMessage* msg) {
	switch (event) {
		case EV_SEND_REQUEST:
			if (packetToSend == NULL) {
				packetToSend =  static_cast<Mac80211Pkt*>(msg);
				EV<<"(1) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff avail]: startTimerBackOff -> BACKOFF." << endl;

				Mac1609_4To80211pControlInfo* ctrlInfo = dynamic_cast<Mac1609_4To80211pControlInfo*>(msg->getControlInfo());
				assert(ctrlInfo);

				currentNumBackoffs = 0;
				switch (ctrlInfo->channel) {
					case type_SCH: {
						currentCW = SCHcontentionParamaters[ctrlInfo->priority]->getCwMin();
						break;
					}
					case type_CCH:	{
						currentCW = CCHcontentionParamaters[ctrlInfo->priority]->getCwMin();
						break;
					}
					default: {
						opp_error("Packet was neither a CCH or a SCH packet");
						break;
					}
				}

				DBG << "Idle when packet was received from upper layer. Starting AIFS. CurrentCW is " << currentCW << std::endl;

				updateMacState(CCA_3);
				startTimer(TIMER_CCA);

			}
			else {
				DBG << "Received a packet although it already has one. This should never ever happen. Packet dropped.";
				assert(false);
			}
			break;
		case EV_BROADCAST_RECEIVED:
			DBG << "(23) FSM State IDLE_1, EV_BROADCAST_RECEIVED: Nothing to do." << endl;
			sendUp(msg);
			break;
		default:
			fsmError(event, msg);
			break;
	}
}

void Mac80211p::updateStatusBackoff(t_mac_event event, cMessage* msg) {
	switch (event) {
		case EV_TIMER_BACKOFF:
			EV<< "(2) FSM State BACKOFF, EV_TIMER_BACKOFF:"
			  << " starting CCA timer." << endl;
			startTimer(TIMER_CCA);
			updateMacState(CCA_3);
			break;
		case EV_FRAME_RECEIVED:
			DBG << "(28) FSM State BACKOFF, EV_FRAME_RECEIVED:";
			sendUp(msg);
			break;
		case EV_BROADCAST_RECEIVED:
			DBG << "(29) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
			    << "sending frame up and resuming normal operation." <<endl;
			sendUp(msg);
			break;
		default:
			fsmError(event, msg);
			break;
	}
}

void Mac80211p::attachSignal(Mac80211Pkt* mac, simtime_t startTime, double frequency) {

	int macPktlen = mac->getBitLength();
	simtime_t duration =
	    PHY_HDR_PREAMBLE_DURATION +
	    PHY_HDR_PLCPSIGNAL_DURATION +
	    ((macPktlen + PHY_HDR_PSDU_HEADER_LENGTH)/bitrate);

	Signal* s = createSignal(startTime, duration, txPower, bitrate, frequency);
	MacToPhyControlInfo* cinfo = new MacToPhyControlInfo(s);

	mac->setControlInfo(cinfo);
}

Signal* Mac80211p::createSignal(simtime_t start, simtime_t length, double power, double bitrate, double frequency) {
	simtime_t end = start + length;
	//create signal with start at current simtime and passed length
	Signal* s = new Signal(start, length);

	//create and set tx power mapping
	ConstMapping* txPowerMapping = createSingleFrequencyMapping(start, end, frequency, 5.0e6, power);
	s->setTransmissionPower(txPowerMapping);

	Mapping* bitrateMapping = MappingUtils::createMapping(DimensionSet::timeDomain, Mapping::STEPS);

	Argument pos(start);
	bitrateMapping->setValue(pos, bitrate);

	pos.setTime(phyHeaderLength / bitrate);
	bitrateMapping->setValue(pos, bitrate);

	s->setBitrate(bitrateMapping);

	return s;
}

void Mac80211p::updateStatusCCA(t_mac_event event, cMessage* msg) {
	switch (event) {
		case EV_TIMER_CCA: {
			EV<< "(25) FSM State CCA_3, EV_TIMER_CCA" << endl;
			bool isIdle = phy->getChannelState().isIdle();
			bool guardActive= _1609_Mac->guardActive();
			DBG << "AIFS period over. Channelbusy: " << !isIdle << " Guard active: " << guardActive << std::endl;
			if (isIdle && !guardActive) {

				DBG << "(3) FSM State CCA_3, EV_TIMER_CCA, [Channel Idle]: -> TRANSMITFRAME_4." << endl;
				updateMacState(TRANSMITFRAME_4);
				phy->setRadioState(Radio::TX);
				Mac80211Pkt* mac = check_and_cast<Mac80211Pkt*>(packetToSend->dup());

				Mac1609_4To80211pControlInfo* ctrlInfo = dynamic_cast<Mac1609_4To80211pControlInfo*>(packetToSend->getControlInfo());
				assert(ctrlInfo);

				mac->removeControlInfo();

				DBG << "Creating signal for access category (priority) " << ctrlInfo->priority << " packet for frequency " << ctrlInfo->frequency << std::endl;

				attachSignal(mac, simTime()+RADIODELAY_11P, ctrlInfo->frequency);
				MacToPhyControlInfo* phyInfo = dynamic_cast<MacToPhyControlInfo*>(mac->getControlInfo());
				assert(phyInfo);

				double timeLeft = _1609_Mac->timeLeft();
				double sendingDuration = RADIODELAY_11P + ((phyHeaderLength+mac->getBitLength())/bitrate);

				//check if theres enough time for the frame to be transmitted in current sch/cch-interval
				if (timeLeft > sendingDuration) {
					// give time for the radio to be in Tx state before transmitting
					sendDelayed(mac, RADIODELAY_11P, lowerLayerOut);
					statsSentPackets++;
				}
				else {   //not enough time left now
					DBG << "This packet cannot be send in this slot. deleting it from queue (Should not happen here) " << std::endl;

					executeMac(EV_FRAME_TRANSMITTED, msg);

					sendControlUp(new cMessage("Not enough time left", Mac80211pToMac1609_4Interface::INTERVAL_TO_SHORT));
					delete(mac);
				}
			}
			else {
				// Channel was busy. increment backoff
				DBG << "(7) FSM State CCA_3, EV_TIMER_CCA, [Channel Busy]: "
				    << " increment counters." << endl;
				currentNumBackoffs++;

				// decide if we go for another backoff or if we drop the frame.
				if (currentNumBackoffs> macMaxCSMABackoffs) {
					// drop the frame
					DBG << "Tried " << currentNumBackoffs << " backoffs, all reported a busy "
					    << "channel. Dropping the packet." << std::endl;
					cMessage* mac = packetToSend;
					packetToSend = NULL;

					txAttempts = 0;
					statsDroppedPackets++;

					mac->setName("MAC ERROR");
					mac->setKind(Mac80211pToMac1609_4Interface::PACKET_DROPPED);

					sendControlUp(mac);

					updateMacState(IDLE_1);
				}
				else {
					DBG << "Backoff attempt Nr. " << currentNumBackoffs << ". Starting backoff timer" << std::endl;
					updateMacState(BACKOFF_2);
					startTimer(TIMER_BACKOFF);
				}
			}
			break;
		}
		case EV_FRAME_RECEIVED:
			DBG << "(26) FSM State CCA_3, EV_FRAME_RECEIVED:";
			sendUp(msg);
			break;
		case EV_BROADCAST_RECEIVED:
			DBG << "(24) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
			    << " Nothing to do." << endl;
			sendUp(msg);
			break;
		default:
			fsmError(event, msg);
			break;
	}
}

void Mac80211p::updateStatusTransmitFrame(t_mac_event event, cMessage* msg) {
	if (event == EV_FRAME_TRANSMITTED) {
		currentNumBackoffs = 0;
		Mac80211Pkt* packet = packetToSend;
		phy->setRadioState(Radio::RX);
		packetToSend = NULL;
		delete packet;
		updateMacState(IDLE_1);
	}
	else {
		fsmError(event, msg);
	}
}

void Mac80211p::updateStatusNotIdle(cMessage* msg) {
	//this should never happen because upper layer only hands down packets if we told it to
	assert(false);
}

/**
 * Updates state machine.
 */
void Mac80211p::executeMac(t_mac_event event, cMessage* msg) {

	if (macState != IDLE_1 && event == EV_SEND_REQUEST) {
		updateStatusNotIdle(msg);
	}
	else {
		switch (macState) {
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
			default:
				DBG << "Error in CSMA FSM: an unknown state has been reached. macState=" << macState << endl;
				opp_error("802.11 FSM received an unknown event");
				break;
		}
	}
}

void Mac80211p::updateMacState(t_mac_states newMacState) {
	macState = newMacState;
}

/*
 * Called by the FSM machine when an unknown transition is requested.
 */
void Mac80211p::fsmError(t_mac_event event, cMessage* msg) {
	DBG << "FSM Error ! In state " << macState << ", received unknown event:" << event << "." << endl;
	if (msg != NULL)
		delete msg;
	opp_error("802.11 FSM received an unknown event");
}

void Mac80211p::startTimer(t_mac_timer timer) {

	Mac1609_4To80211pControlInfo* ctrlInfo = dynamic_cast<Mac1609_4To80211pControlInfo*>(packetToSend->getControlInfo());
	assert(ctrlInfo);

	double bitLength = packetToSend->getBitLength();
	double timeLeft = _1609_Mac->timeLeft();
	double AIFSDelay = calculateAIFS();

	if (timer == TIMER_BACKOFF) {
		DBG << "Backoff Timer for packet with priority " << ctrlInfo->priority << " received. " << std::endl;
		double backOff = 0;
		if (_1609_Mac->guardActive()) {
			DBG << "Guard is Active." << std::endl;
			/*
			 * Performance improvement: instead of going to backoff multiple times until the guard is over, just set cw to CW_max
			 * and come back after the guard finished
			 */
			switch (ctrlInfo->channel) {
				case type_SCH: {
					currentCW = SCHcontentionParamaters[ctrlInfo->priority]->getCwMax();
					break;
				}
				case type_CCH: {
					currentCW = CCHcontentionParamaters[ctrlInfo->priority]->getCwMax();
					break;
				}
			}
			backOff = calculateBackoff() + _1609_Mac->timeLeftTillGuardOver();
		}
		else {
			//Guard is not active
			backOff = calculateBackoff();
		}
		DBG << "Backoff value is: " << backOff << ". Current contention window is " << currentCW <<  std::endl;
		double sendingDuration = backOff + AIFSDelay + RADIODELAY_11P + ((phyHeaderLength+bitLength)/bitrate);
		if (timeLeft < sendingDuration) {
			DBG << "This Packet cannot be sent in this slot. Removing it from queue." << std::endl;

			updateMacState(TRANSMITFRAME_4);
			phy->setRadioState(Radio::TX);
			statsNumTooLittleTime++;

			executeMac(EV_FRAME_TRANSMITTED, packetToSend);
			sendControlUp(new cMessage("Not enough time left", Mac80211pToMac1609_4Interface::INTERVAL_TO_SHORT));
		}
		else {
			DBG << "This Packet can be sent in this slot. Scheduling backoff " << std::endl;
			scheduleAt(simTime() + backOff, backoffTimer);
		}

	}
	else if (timer == TIMER_CCA) {

		DBG << "timerCCA Received. Starting AIFS for packet with priority " << ctrlInfo->priority << ". AIFS is: " << AIFSDelay << std::endl;

		double sendingDuration = AIFSDelay + RADIODELAY_11P + ((phyHeaderLength+bitLength)/bitrate);
		DBG << "This Packet needs " << sendingDuration << "s. We have " << timeLeft << "s left." << std::endl;
		if (timeLeft < sendingDuration) {
			DBG << "csmaMod This Packet cannot be sent in this slot. Removing it from queue at " << simTime().dbl() << std::endl;

			updateMacState(TRANSMITFRAME_4);
			phy->setRadioState(Radio::TX);
			statsNumTooLittleTime++;

			executeMac(EV_FRAME_TRANSMITTED, packetToSend);
			sendControlUp(new cMessage("Not enough time left", Mac80211pToMac1609_4Interface::INTERVAL_TO_SHORT));
		}
		else {

			DBG << "This Packet can be sent in this slot. Scheduling AIFS" << std::endl;
			scheduleAt(simTime()+AIFSDelay, ccaTimer);
		}
	}
	else {
		DBG << "Unknown timer requested to start:" << timer << endl;
	}
}

double Mac80211p::calculateAIFS() const {
	Mac1609_4To80211pControlInfo* ctrlInfo = dynamic_cast<Mac1609_4To80211pControlInfo*>(packetToSend->getControlInfo());
	assert(ctrlInfo);
	t_channel channelType = ctrlInfo->channel;
	int priority = ctrlInfo->priority;
	int aifsn;
	switch (channelType) {
		case type_SCH: {
			aifsn = SCHcontentionParamaters[priority]->getAifsn();
			break;
		}
		case type_CCH: {
			aifsn = CCHcontentionParamaters[priority]->getAifsn();
			break;
		}
	}
	return aifsn * SLOTLENGTH_11P + SIFS_11P; //according to 802.11p standard
}

double Mac80211p::calculateBackoff() {

	double backoffTime;
	int slots;

	Mac1609_4To80211pControlInfo* ctrlInfo = dynamic_cast<Mac1609_4To80211pControlInfo*>(packetToSend->getControlInfo());
	assert(ctrlInfo);
	t_channel channelType = ctrlInfo->channel;
	int priority = ctrlInfo->priority;

	std::vector<ContentionParameters*> conPar;
	switch (channelType) {
		case type_SCH:	{
			conPar = SCHcontentionParamaters;
			break;
		}
		case type_CCH: {
			conPar = CCHcontentionParamaters;
			break;
		}
	}

	slots = intuniform(0,currentCW);
	currentCW = std::min(currentCW * 2,conPar[priority]->getCwMax());

	backoffTime = slots * SLOTLENGTH_11P;

	statsNumBackoffs++;
	statsTotalBackoffDuration += backoffTime;

	return backoffTime;
}

/*
 * Binds timers to events and executes FSM.
 */
void Mac80211p::handleSelfMsg(cMessage* msg) {
	EV<< "timer routine." << endl;
	if (msg==backoffTimer)
		executeMac(EV_TIMER_BACKOFF, msg);
	else if (msg==ccaTimer)
		executeMac(EV_TIMER_CCA, msg);
	else
		DBG << "CSMA Error: unknown timer fired:" << msg << endl;
}

/**
 * Compares the address of this Host with the destination address in
 * the received frame. Generates the corresponding event.
 */
void Mac80211p::handleLowerMsg(cMessage* msg) {
	Mac80211Pkt* macPkt = static_cast<Mac80211Pkt*>(msg);
	long dest = macPkt->getDestAddr();

	DBG << "Received frame name= " << macPkt->getName()
	    << ", myState=" << macState << " src=" << macPkt->getSrcAddr()
	    << " dst=" << macPkt->getDestAddr() << " myAddr="
	    << myMacAddress << endl;

	if (macPkt->getDestAddr() == myMacAddress) {
		DBG << "Received a data packet addressed to me." << endl;
		statsReceivedPackets++;
		executeMac(EV_FRAME_RECEIVED, macPkt);
	}
	else if (dest == LAddress::L2BROADCAST) {
		statsReceivedBroadcasts++;
		executeMac(EV_BROADCAST_RECEIVED, macPkt);
	}
	else {
		DBG << "Packet not for me, deleting...\n";
		delete macPkt;
	}
}

void Mac80211p::handleLowerControl(cMessage* msg) {
	if (msg->getKind() == MacToPhyInterface::TX_OVER) {
		DBG << "Phylayer said packet was sent at " << simTime().dbl() << std::endl;
		sendControlUp(msg->dup());
		executeMac(EV_FRAME_TRANSMITTED, msg);
	}
	else if (msg->getKind() == BaseDecider::PACKET_DROPPED) {
		DBG << "Phylayer said packet was dropped" << std::endl;
	}
	else if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
		DBG << "Phylayer said radio switching is done" << std::endl;
	}
	else if (msg->getKind() == Decider80211p::BITERROR) {
		statsLostPackets++;
		DBG << "A packet was not received due to biterrors" << std::endl;
	}
	else {
		DBG << "Invalid control message type (type=NOTHING) : name="
		    << msg->getName() << " modulesrc="
		    << msg->getSenderModule()->getFullPath()
		    << "." << endl;
		assert(false);
	}
	delete msg;
}

void Mac80211p::checkBitrate(int bitrate) const {
	for (unsigned int i = 0; i < sizeof(BITRATES_80211P); i++) {
		if (bitrate == BITRATES_80211P[i]) return;
	}
	opp_error("Chosen Bitrate is not valid for 802.11p: Valid rates are: 3Mbps, 4.5Mbps, 6Mbps, 9Mbps, 12Mbps, 18Mbps, 24Mbps and 27Mbps. Please adjust your omnetpp.ini file accordingly.");
}
