//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
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

#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include <iterator>

#include "veins/modules/phy/DeciderResult80211.h"
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/modules/messages/PhyControlMessage_m.h"
#include "veins/modules/messages/AckTimeOutMessage_m.h"

using std::unique_ptr;
using omnetpp::simtime_t;
using omnetpp::simTime;

#if OMNETPP_VERSION >= 0x500
#define OWNER owner->
#else
#define OWNER
#endif

#define DBG_MAC EV
//#define DBG_MAC std::cerr << "[" << simTime().raw() << "] " << myId << " "

Define_Module(Mac1609_4);

void Mac1609_4::initialize(int stage) {
	BaseMacLayer::initialize(stage);
	if (stage == 0) {

		phy11p = FindModule<Mac80211pToPhy11pInterface*>::findSubModule(
		             getParentModule());
		assert(phy11p);

		//this is required to circumvent double precision issues with constants from CONST80211p.h
		assert(simTime().getScaleExp() == -12);

		sigChannelBusy = registerSignal("sigChannelBusy");
		sigCollision = registerSignal("sigCollision");

		txPower = par("txPower").doubleValue();
		bitrate = par("bitrate").longValue();
		n_dbps = 0;
		setParametersForBitrate(bitrate);

		// unicast parameters
		dot11RTSThreshold = par("dot11RTSThreshold");
		dot11ShortRetryLimit = par("dot11ShortRetryLimit");
		dot11LongRetryLimit = par("dot11LongRetryLimit");
		ackLength = par("ackLength");
		useAcks = par("useAcks").boolValue();
		ackErrorRate = par("ackErrorRate").doubleValue();
		rxStartIndication = false;
		ignoreChannelState = false;
		waitUntilAckRXorTimeout = false;
		stopIgnoreChannelStateMsg = new cMessage("ChannelStateMsg");

		//mac-adresses
		myMacAddress = getParentModule()->getParentModule()->getIndex();
		myId = getParentModule()->getParentModule()->getFullPath();
		//create frequency mappings
		frequency.insert(std::pair<int, double>(Channels::CRIT_SOL, 5.86e9));
		frequency.insert(std::pair<int, double>(Channels::SCH1, 5.87e9));
		frequency.insert(std::pair<int, double>(Channels::SCH2, 5.88e9));
		frequency.insert(std::pair<int, double>(Channels::CCH, 5.89e9));
		frequency.insert(std::pair<int, double>(Channels::SCH3, 5.90e9));
		frequency.insert(std::pair<int, double>(Channels::SCH4, 5.91e9));
		frequency.insert(std::pair<int, double>(Channels::HPPS, 5.92e9));

		//create two edca systems

		myEDCA[type_CCH] = std::unique_ptr<EDCA>(new EDCA(this, type_CCH,par("queueSize").longValue()));
		myEDCA[type_CCH]->myId = myId;
		myEDCA[type_CCH]->myId.append(" CCH");
		myEDCA[type_CCH]->createQueue(2,(((CWMIN_11P+1)/4)-1),(((CWMIN_11P +1)/2)-1),AC_VO);
		myEDCA[type_CCH]->createQueue(3,(((CWMIN_11P+1)/2)-1),CWMIN_11P,AC_VI);
		myEDCA[type_CCH]->createQueue(6,CWMIN_11P,CWMAX_11P,AC_BE);
		myEDCA[type_CCH]->createQueue(9,CWMIN_11P,CWMAX_11P,AC_BK);

		myEDCA[type_SCH] = std::unique_ptr<EDCA>(new EDCA(this, type_SCH,par("queueSize").longValue()));
		myEDCA[type_SCH]->myId = myId;
		myEDCA[type_SCH]->myId.append(" SCH");
		myEDCA[type_SCH]->createQueue(2,(((CWMIN_11P+1)/4)-1),(((CWMIN_11P +1)/2)-1),AC_VO);
		myEDCA[type_SCH]->createQueue(3,(((CWMIN_11P+1)/2)-1),CWMIN_11P,AC_VI);
		myEDCA[type_SCH]->createQueue(6,CWMIN_11P,CWMAX_11P,AC_BE);
		myEDCA[type_SCH]->createQueue(9,CWMIN_11P,CWMAX_11P,AC_BK);

		useSCH = par("useServiceChannel").boolValue();
		if (useSCH) {
			if (useAcks) throw cRuntimeError("Unicast model does not support channel switching");
			//set the initial service channel
			switch (par("serviceChannel").longValue()) {
				case 1: mySCH = Channels::SCH1; break;
				case 2: mySCH = Channels::SCH2; break;
				case 3: mySCH = Channels::SCH3; break;
				case 4: mySCH = Channels::SCH4; break;
				default: throw cRuntimeError("Service Channel must be between 1 and 4"); break;
			}
		}

		headerLength = par("headerLength");

		nextMacEvent = new cMessage("next Mac Event");

		if (useSCH) {
			uint64_t currenTime = simTime().raw();
			uint64_t switchingTime = SWITCHING_INTERVAL_11P.raw();
			double timeToNextSwitch = (double)(switchingTime
							   - (currenTime % switchingTime)) / simTime().getScale();
			if ((currenTime / switchingTime) % 2 == 0) {
				setActiveChannel(type_CCH);
			}
			else {
				setActiveChannel(type_SCH);
			}

			// channel switching active
			nextChannelSwitch = new cMessage("Channel Switch");
			//add a little bit of offset between all vehicles, but no more than syncOffset
			simtime_t offset = dblrand() * par("syncOffset").doubleValue();
			scheduleAt(simTime() + offset + timeToNextSwitch, nextChannelSwitch);
		}
		else {
			// no channel switching
			nextChannelSwitch = 0;
			setActiveChannel(type_CCH);
		}


		//stats
		statsReceivedPackets = 0;
		statsReceivedBroadcasts = 0;
		statsSentPackets = 0;
		statsSentAcks = 0;
		statsTXRXLostPackets = 0;
		statsSNIRLostPackets = 0;
		statsDroppedPackets = 0;
		statsNumTooLittleTime = 0;
		statsNumInternalContention = 0;
		statsNumBackoff = 0;
		statsSlotsBackoff = 0;
		statsTotalBusyTime = 0;

		idleChannel = true;
		lastBusy = simTime();
		channelIdle(true);
	}
}

void Mac1609_4::handleSelfMsg(cMessage* msg) {
	if (msg == stopIgnoreChannelStateMsg) {
		ignoreChannelState = false;
		return;
	}

	if (AckTimeOutMessage* ackTimeOutMsg = dynamic_cast<AckTimeOutMessage*>(msg)) {
		handleAckTimeOut(ackTimeOutMsg);
		return;
	}

	if (msg == nextChannelSwitch) {
		ASSERT(useSCH);

		scheduleAt(simTime() + SWITCHING_INTERVAL_11P, nextChannelSwitch);

		switch (activeChannel) {
			case type_CCH:
				DBG_MAC << "CCH --> SCH" << std::endl;
				channelBusySelf(false);
				setActiveChannel(type_SCH);
				channelIdle(true);
				phy11p->changeListeningFrequency(frequency[mySCH]);
				break;
			case type_SCH:
				DBG_MAC << "SCH --> CCH" << std::endl;
				channelBusySelf(false);
				setActiveChannel(type_CCH);
				channelIdle(true);
				phy11p->changeListeningFrequency(frequency[Channels::CCH]);
				break;
		}
		//schedule next channel switch in 50ms

	}
	else if (msg ==  nextMacEvent) {

		//we actually came to the point where we can send a packet
		channelBusySelf(true);
		WaveShortMessage* pktToSend = myEDCA[activeChannel]->initiateTransmit(lastIdle);
		ASSERT(pktToSend);

		lastAC = mapUserPriority(pktToSend->getUserPriority());
		lastWSM = pktToSend;

		DBG_MAC << "MacEvent received. Trying to send packet with priority" << lastAC << std::endl;

		//send the packet
		Mac80211Pkt* mac = new Mac80211Pkt(pktToSend->getName(), pktToSend->getKind());
		if (pktToSend->getRecipientAddress() != -1) {
			mac->setDestAddr(pktToSend->getRecipientAddress());
		} else {
			mac->setDestAddr(LAddress::L2BROADCAST());
		}
		mac->setSrcAddr(myMacAddress);
		mac->encapsulate(pktToSend->dup());

		enum PHY_MCS mcs;
		double txPower_mW;
		uint64_t datarate;
		PhyControlMessage *controlInfo = dynamic_cast<PhyControlMessage *>(pktToSend->getControlInfo());
		if (controlInfo) {
			//if MCS is not specified, just use the default one
			mcs = (enum PHY_MCS)controlInfo->getMcs();
			if (mcs != MCS_DEFAULT) {
				datarate = getOfdmDatarate(mcs, BW_OFDM_10_MHZ);
			}
			else {
				datarate = bitrate;
			}
			//apply the same principle to tx power
			txPower_mW = controlInfo->getTxPower_mW();
			if (txPower_mW < 0) {
				txPower_mW = txPower;
			}
		}
		else {
			mcs = MCS_DEFAULT;
			txPower_mW = txPower;
			datarate = bitrate;
		}

		simtime_t sendingDuration = RADIODELAY_11P + getFrameDuration(mac->getBitLength(), mcs);
		DBG_MAC << "Sending duration will be" << sendingDuration << std::endl;
		if ((!useSCH) || (timeLeftInSlot() > sendingDuration)) {
			if (useSCH) DBG_MAC << " Time in this slot left: " << timeLeftInSlot() << std::endl;

			double freq = (activeChannel == type_CCH) ? frequency[Channels::CCH] : frequency[mySCH];

			DBG_MAC << "Sending a Packet. Frequency " << freq << " Priority" << lastAC << std::endl;
			sendFrame(mac, RADIODELAY_11P, freq, datarate, txPower_mW);

			// schedule ack timeout for unicast packets
			if (pktToSend->getRecipientAddress() != -1 && useAcks) {
				waitUntilAckRXorTimeout = true;
				// PHY-RXSTART.indication should be received within ackWaitTime
				// sifs + slot + rx_delay: see 802.11-2012 9.3.2.8 (32us + 13us + 49us = 94us)
				simtime_t ackWaitTime(94, SIMTIME_US);
				// update id in the retransmit timer
				myEDCA[activeChannel]->myQueues[lastAC].ackTimeOut->setWsmId(pktToSend->getTreeId());
				simtime_t timeOut = sendingDuration + ackWaitTime;
				scheduleAt(simTime() + timeOut, myEDCA[activeChannel]->myQueues[lastAC].ackTimeOut);
			}
		}
		else {   //not enough time left now
			DBG_MAC << "Too little Time left. This packet cannot be send in this slot." << std::endl;
			statsNumTooLittleTime++;
			//revoke TXOP
			myEDCA[activeChannel]->revokeTxOPs();
			delete mac;
			channelIdle();
			//do nothing. contention will automatically start after channel switch
		}
	}
}

void Mac1609_4::handleUpperControl(cMessage* msg) {
	assert(false);
}

void Mac1609_4::handleUpperMsg(cMessage* msg) {

	WaveShortMessage* thisMsg;
	if ((thisMsg = dynamic_cast<WaveShortMessage*>(msg)) == NULL) {
		error("WaveMac only accepts WaveShortMessages");
	}

	t_access_category ac = mapUserPriority(thisMsg->getUserPriority());

	DBG_MAC << "Received a message from upper layer for channel "
	        << thisMsg->getChannelNumber() << " Access Category (Priority):  "
	        << ac << std::endl;

	t_channel chan;

	if (thisMsg->getChannelNumber() == Channels::CCH) {
	    chan = type_CCH;
	}
	else {
		ASSERT(useSCH);
		thisMsg->setChannelNumber(mySCH);
		chan = type_SCH;
	}

	int num = myEDCA[chan]->queuePacket(ac,thisMsg);

	//packet was dropped in Mac
	if (num == -1) {
		statsDroppedPackets++;
		return;
	}

	//if this packet is not at the front of a new queue we dont have to reevaluate times
	DBG_MAC << "sorted packet into queue of EDCA " << chan << " this packet is now at position: " << num << std::endl;

	if (chan == activeChannel) {
		DBG_MAC << "this packet is for the currently active channel" << std::endl;
	}
	else {
		DBG_MAC << "this packet is NOT for the currently active channel" << std::endl;
	}

	if (num == 1 && idleChannel == true && chan == activeChannel) {

		simtime_t nextEvent = myEDCA[chan]->startContent(lastIdle,guardActive());

		if (nextEvent != -1) {
			if ((!useSCH) || (nextEvent <= nextChannelSwitch->getArrivalTime())) {
				if (nextMacEvent->isScheduled()) {
					cancelEvent(nextMacEvent);
				}
				scheduleAt(nextEvent,nextMacEvent);
				DBG_MAC << "Updated nextMacEvent:" << nextMacEvent->getArrivalTime().raw() << std::endl;
			}
			else {
				DBG_MAC << "Too little time in this interval. Will not schedule nextMacEvent" << std::endl;
				//it is possible that this queue has an txop. we have to revoke it
				myEDCA[activeChannel]->revokeTxOPs();
				statsNumTooLittleTime++;
			}
		}
		else {
			cancelEvent(nextMacEvent);
		}
	}
	if (num == 1 && idleChannel == false && myEDCA[chan]->myQueues[ac].currentBackoff == 0 && chan == activeChannel) {
		myEDCA[chan]->backoff(ac);
	}

}

void Mac1609_4::handleLowerControl(cMessage* msg) {
	if (msg->getKind() == MacToPhyInterface::PHY_RX_START) {
		rxStartIndication = true;
	} else if (msg->getKind() == MacToPhyInterface::PHY_RX_END_WITH_SUCCESS) {
		// PHY_RX_END_WITH_SUCCESS will get packet soon! Nothing to do here
	} else if (msg->getKind() == MacToPhyInterface::PHY_RX_END_WITH_FAILURE) {
		// RX failed at phy. Time to retransmit
		phy11p->notifyMacAboutRxStart(false);
		rxStartIndication = false;
		handleRetransmit(lastAC);
	} else if (msg->getKind() == MacToPhyInterface::TX_OVER) {

		DBG_MAC << "Successfully transmitted a packet on " << lastAC << std::endl;

		phy->setRadioState(Radio::RX);

		if (!dynamic_cast<Mac80211Ack*>(lastMac.get())) {
			//message was sent
			//update EDCA queue. go into post-transmit backoff and set cwCur to cwMin
			myEDCA[activeChannel]->postTransmit(lastAC, lastWSM, useAcks);
		}
		//channel just turned idle.
		//don't set the chan to idle. the PHY layer decides, not us.

		if (guardActive()) {
			throw cRuntimeError("We shouldnt have sent a packet in guard!");
		}
	}
	else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_BUSY) {
		channelBusy();
	}
	else if (msg->getKind() == Mac80211pToPhy11pInterface::CHANNEL_IDLE) {
		// Decider80211p::processSignalEnd() sends up the received packet to MAC followed by control message CHANNEL_IDLE in the same timestamp.
		// If we received a unicast frame (first event scheduled by Decider), MAC immediately schedules an ACK message and wants to switch the radio to TX mode.
		// So, the notification for channel idle from phy is undesirable and we skip it here.
		// After ACK TX is over, PHY will inform the channel status again.
		if (ignoreChannelState) {
			// Skipping channelidle because we are about to send an ack regardless of the channel state
		} else {
			channelIdle();
		}
	}
	else if (msg->getKind() == Decider80211p::BITERROR || msg->getKind() == Decider80211p::COLLISION) {
		statsSNIRLostPackets++;
		DBG_MAC << "A packet was not received due to biterrors" << std::endl;
	}
	else if (msg->getKind() == Decider80211p::RECWHILESEND) {
		statsTXRXLostPackets++;
		DBG_MAC << "A packet was not received because we were sending while receiving" << std::endl;
	}
	else if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER) {
		DBG_MAC << "Phylayer said radio switching is done" << std::endl;
	}
	else if (msg->getKind() == BaseDecider::PACKET_DROPPED) {
		phy->setRadioState(Radio::RX);
		DBG_MAC << "Phylayer said packet was dropped" << std::endl;
	}
	else {
		DBG_MAC << "Invalid control message type (type=NOTHING) : name=" << msg->getName() << " modulesrc=" << msg->getSenderModule()->getFullPath() << "." << std::endl;
		assert(false);
	}

	if (msg->getKind() == Decider80211p::COLLISION) {
		emit(sigCollision, true);
	}

	delete msg;
}

void Mac1609_4::setActiveChannel(t_channel state) {
	activeChannel = state;
	assert(state == type_CCH || (useSCH && state == type_SCH));
}

void Mac1609_4::finish() {
	for (auto&& p : myEDCA) {
		statsNumInternalContention += p.second->statsNumInternalContention;
		statsNumBackoff += p.second->statsNumBackoff;
		statsSlotsBackoff += p.second->statsSlotsBackoff;
	}

	recordScalar("ReceivedUnicastPackets", statsReceivedPackets);
	recordScalar("ReceivedBroadcasts", statsReceivedBroadcasts);
	recordScalar("SentPackets", statsSentPackets);
	recordScalar("SentAcknowledgements", statsSentAcks);
	recordScalar("SNIRLostPackets", statsSNIRLostPackets);
	recordScalar("RXTXLostPackets", statsTXRXLostPackets);
	recordScalar("TotalLostPackets", statsSNIRLostPackets+statsTXRXLostPackets);
	recordScalar("DroppedPacketsInMac", statsDroppedPackets);
	recordScalar("TooLittleTime", statsNumTooLittleTime);
	recordScalar("TimesIntoBackoff", statsNumBackoff);
	recordScalar("SlotsBackoff", statsSlotsBackoff);
	recordScalar("NumInternalContention", statsNumInternalContention);
	recordScalar("totalBusyTime", statsTotalBusyTime.dbl());
}

Mac1609_4::~Mac1609_4() {
	if (nextMacEvent) {
		cancelAndDelete(nextMacEvent);
		nextMacEvent = nullptr;
	}

	if (nextChannelSwitch) {
		cancelAndDelete(nextChannelSwitch);
		nextChannelSwitch = nullptr;
	}

	if (stopIgnoreChannelStateMsg) {
		cancelAndDelete(stopIgnoreChannelStateMsg);
		stopIgnoreChannelStateMsg = nullptr;
	}
};

void Mac1609_4::sendFrame(Mac80211Pkt* frame, simtime_t delay, double frequency, uint64_t datarate, double txPower_mW) {
	phy->setRadioState(Radio::TX); // give time for the radio to be in Tx state before transmitting

	delay = std::max(delay, RADIODELAY_11P); // wait at least for the radio to switch

	attachSignal(frame, simTime() + delay, frequency, datarate, txPower_mW);
	MacToPhyControlInfo* phyInfo = dynamic_cast<MacToPhyControlInfo*>(frame->getControlInfo());
	ASSERT(phyInfo);

	lastMac.reset(frame->dup());
	sendDelayed(frame, delay, lowerLayerOut);

	if (dynamic_cast<Mac80211Ack*>(frame)) {
		statsSentAcks += 1;
	} else {
		statsSentPackets += 1;
	}
}

void Mac1609_4::attachSignal(Mac80211Pkt* mac, simtime_t startTime, double frequency, uint64_t datarate, double txPower_mW) {

	simtime_t duration = getFrameDuration(mac->getBitLength());

	Signal* s = createSignal(startTime, duration, txPower_mW, datarate, frequency);
	MacToPhyControlInfo* cinfo = new MacToPhyControlInfo(s);

	mac->setControlInfo(cinfo);
}

Signal* Mac1609_4::createSignal(simtime_t start, simtime_t length, double power, uint64_t bitrate, double frequency) {
	simtime_t end = start + length;
	//create signal with start at current simtime and passed length
	Signal* s = new Signal(start, length);

	//create and set tx power mapping
	ConstMapping* txPowerMapping = createSingleFrequencyMapping(start, end, frequency, 5.0e6, power);
	s->setTransmissionPower(txPowerMapping);

	Mapping* bitrateMapping = MappingUtils::createMapping(DimensionSet::timeDomain(), Mapping::STEPS);

	Argument pos(start);
	bitrateMapping->setValue(pos, bitrate);

	pos.setTime(phyHeaderLength / bitrate);
	bitrateMapping->setValue(pos, bitrate);

	s->setBitrate(bitrateMapping);

	return s;
}

/* checks if guard is active */
bool Mac1609_4::guardActive() const {
	if (!useSCH) return false;
	if (simTime().dbl() - nextChannelSwitch->getSendingTime() <= GUARD_INTERVAL_11P)
		return true;
	return false;
}

/* returns the time until the guard is over */
simtime_t Mac1609_4::timeLeftTillGuardOver() const {
	ASSERT(useSCH);
	simtime_t sTime = simTime();
	if (sTime - nextChannelSwitch->getSendingTime() <= GUARD_INTERVAL_11P) {
		return GUARD_INTERVAL_11P
		       - (sTime - nextChannelSwitch->getSendingTime());
	}
	else
		return 0;
}

/* returns the time left in this channel window */
simtime_t Mac1609_4::timeLeftInSlot() const {
	ASSERT(useSCH);
	return nextChannelSwitch->getArrivalTime() - simTime();
}

/* Will change the Service Channel on which the mac layer is listening and sending */
void Mac1609_4::changeServiceChannel(int cN) {
	ASSERT(useSCH);
	if (cN != Channels::SCH1 && cN != Channels::SCH2 && cN != Channels::SCH3 && cN != Channels::SCH4) {
		throw cRuntimeError("This Service Channel doesnt exit: %d",cN);
	}

	mySCH = cN;

	if (activeChannel == type_SCH) {
		//change to new chan immediately if we are in a SCH slot,
		//otherwise it will switch to the new SCH upon next channel switch
		phy11p->changeListeningFrequency(frequency[mySCH]);
	}
}

void Mac1609_4::setTxPower(double txPower_mW) {
	txPower = txPower_mW;
}
void Mac1609_4::setMCS(enum PHY_MCS mcs) {
	ASSERT2(mcs != MCS_DEFAULT, "invalid MCS selected");
	bitrate = getOfdmDatarate(mcs, BW_OFDM_10_MHZ);
	setParametersForBitrate(bitrate);
}

void Mac1609_4::setCCAThreshold(double ccaThreshold_dBm) {
	phy11p->setCCAThreshold(ccaThreshold_dBm);
}

void Mac1609_4::handleLowerMsg(cMessage* msg) {
	Mac80211Pkt* macPkt = check_and_cast<Mac80211Pkt*>(msg);

	//pass information about received frame to the upper layers
	DeciderResult80211 *macRes = check_and_cast<DeciderResult80211 *>(PhyToMacControlInfo::getDeciderResult(msg));
	DeciderResult80211 *res = new DeciderResult80211(*macRes);

	long dest = macPkt->getDestAddr();

	DBG_MAC << "Received frame name= " << macPkt->getName()
	        << ", myState=" << " src=" << macPkt->getSrcAddr()
	        << " dst=" << macPkt->getDestAddr() << " myAddr="
	        << myMacAddress << std::endl;

	if (dest == myMacAddress) {
		if (auto* ack = dynamic_cast<Mac80211Ack*>(macPkt)) {
			if (useAcks) {
				handleAck(ack);
			}
		} else {
			unique_ptr<WaveShortMessage> wsm(check_and_cast<WaveShortMessage*>(macPkt->decapsulate()));
			wsm->setControlInfo(new PhyToMacControlInfo(res));
			handleUnicast(std::move(wsm));
		}
	} else if (dest == LAddress::L2BROADCAST()) {
		statsReceivedBroadcasts++;
		unique_ptr<WaveShortMessage> wsm(check_and_cast<WaveShortMessage*>(macPkt->decapsulate()));
		wsm->setControlInfo(new PhyToMacControlInfo(res));
		sendUp(wsm.release());
	} else {
		DBG_MAC << "Packet not for me" << std::endl;
	}
	delete macPkt;

	if (rxStartIndication) {
		// We have handled/processed the incoming packet
		// Since we reached here, we were expecting an ack but we didnt get it, so retransmission should take place
		phy11p->notifyMacAboutRxStart(false);
		rxStartIndication = false;
		handleRetransmit(lastAC);
	}
}

int Mac1609_4::EDCA::queuePacket(t_access_category ac,WaveShortMessage* msg) {

	if (maxQueueSize && myQueues[ac].queue.size() >= maxQueueSize) {
		delete msg;
		return -1;
	}
	myQueues[ac].queue.push(msg);
	return myQueues[ac].queue.size();
}

void Mac1609_4::EDCA::createQueue(int aifsn, int cwMin, int cwMax,t_access_category ac) {

	if (myQueues.find(ac) != myQueues.end()) {
		throw cRuntimeError("You can only add one queue per Access Category per EDCA subsystem");
	}

	EDCAQueue newQueue(aifsn,cwMin,cwMax,ac);
	myQueues[ac] = newQueue;
}

Mac1609_4::t_access_category Mac1609_4::mapUserPriority(int prio) {
	// Map user priority to access category, based on IEEE Std 802.11-2012, Table 9-1
	switch (prio) {
		case 1: return AC_BK;
		case 2: return AC_BK;
		case 0: return AC_BE;
		case 3: return AC_BE;
		case 4: return AC_VI;
		case 5: return AC_VI;
		case 6: return AC_VO;
		case 7: return AC_VO;
		default: throw cRuntimeError("MacLayer received a packet with unknown priority"); break;
	}
	return AC_VO;
}

WaveShortMessage* Mac1609_4::EDCA::initiateTransmit(simtime_t lastIdle) {

	//iterate through the queues to return the packet we want to send
	WaveShortMessage* pktToSend = NULL;

	simtime_t idleTime = simTime() - lastIdle;

	DBG_MAC << "Initiating transmit at " << simTime() << ". I've been idle since " << idleTime << std::endl;

	// As t_access_category is sorted by priority, we iterate back to front.
	// This realizes the behavior documented in IEEE Std 802.11-2012 Section 9.2.4.2; that is, "data frames from the higher priority AC" win an internal collision.
	// The phrase "EDCAF of higher UP" of IEEE Std 802.11-2012 Section 9.19.2.3 is assumed to be meaningless.
	for (auto iter = myQueues.rbegin(); iter != myQueues.rend(); iter++) {
		if (iter->second.queue.size() != 0 && !iter->second.waitForAck) {
			if (idleTime >= iter->second.aifsn* SLOTLENGTH_11P + SIFS_11P && iter->second.txOP == true) {

				DBG_MAC << "Queue " << iter->first << " is ready to send!" << std::endl;

				iter->second.txOP = false;
				//this queue is ready to send
				if (pktToSend == NULL) {
					pktToSend = iter->second.queue.front();
				}
				else {
					//there was already another packet ready. we have to go increase cw and go into backoff. It's called internal contention and its wonderful

					statsNumInternalContention++;
					iter->second.cwCur = std::min(iter->second.cwMax,(iter->second.cwCur+1)*2-1);
					iter->second.currentBackoff = OWNER intuniform(0,iter->second.cwCur);
					DBG_MAC << "Internal contention for queue " << iter->first  << " : "<< iter->second.currentBackoff << ". Increase cwCur to " << iter->second.cwCur << std::endl;
				}
			}
		}
	}

	if (pktToSend == NULL) {
		throw cRuntimeError("No packet was ready");
	}
	return pktToSend;
}

simtime_t Mac1609_4::EDCA::startContent(simtime_t idleSince,bool guardActive) {

	DBG_MAC << "Restarting contention." << std::endl;

	simtime_t nextEvent = -1;

	simtime_t idleTime = SimTime().setRaw(std::max((int64_t)0,(simTime() - idleSince).raw()));;

	lastStart = idleSince;

	DBG_MAC << "Channel is already idle for:" << idleTime << " since " << idleSince << std::endl;

	//this returns the nearest possible event in this EDCA subsystem after a busy channel

	for (auto&& p : myQueues) {
		auto &accessCategory = p.first;
		auto &edcaQueue = p.second;
		if (edcaQueue.queue.size() != 0 && !edcaQueue.waitForAck) {

			/* 1609_4 says that when attempting to send (backoff == 0) when guard is active, a random backoff is invoked */

			if (guardActive == true && edcaQueue.currentBackoff == 0) {
				//cw is not increased
				edcaQueue.currentBackoff = OWNER intuniform(0,edcaQueue.cwCur);
				statsNumBackoff++;
			}

			simtime_t DIFS = edcaQueue.aifsn * SLOTLENGTH_11P + SIFS_11P;

			//the next possible time to send can be in the past if the channel was idle for a long time, meaning we COULD have sent earlier if we had a packet
			simtime_t possibleNextEvent = DIFS + edcaQueue.currentBackoff * SLOTLENGTH_11P;


			DBG_MAC << "Waiting Time for Queue " << accessCategory <<  ":" << possibleNextEvent << "=" << edcaQueue.aifsn << " * "  << SLOTLENGTH_11P << " + " << SIFS_11P << "+" << edcaQueue.currentBackoff << "*" << SLOTLENGTH_11P << "; Idle time: " << idleTime << std::endl;

			if (idleTime > possibleNextEvent) {
				DBG_MAC << "Could have already send if we had it earlier" << std::endl;
				//we could have already sent. round up to next boundary
				simtime_t base = idleSince + DIFS;
				possibleNextEvent =  simTime() - simtime_t().setRaw((simTime() - base).raw() % SLOTLENGTH_11P.raw()) + SLOTLENGTH_11P;
			}
			else {
				//we are gonna send in the future
				DBG_MAC << "Sending in the future" << std::endl;
				possibleNextEvent =  idleSince + possibleNextEvent;
			}
			nextEvent == -1? nextEvent =  possibleNextEvent : nextEvent = std::min(nextEvent,possibleNextEvent);
		}
	}
	return nextEvent;
}

void Mac1609_4::EDCA::stopContent(bool allowBackoff, bool generateTxOp) {
	//update all Queues

	DBG_MAC << "Stopping Contention at " << simTime().raw() << std::endl;

	simtime_t passedTime = simTime() - lastStart;

	DBG_MAC << "Channel was idle for " << passedTime << std::endl;

	lastStart = -1; //indicate that there was no last start

	for (auto&& p : myQueues) {
		auto &accessCategory = p.first;
		auto &edcaQueue = p.second;
		if ((edcaQueue.currentBackoff != 0 || edcaQueue.queue.size() != 0) && !edcaQueue.waitForAck) {
			//check how many slots we already waited until the chan became busy

			int64_t oldBackoff = edcaQueue.currentBackoff;

			std::string info;
			if (passedTime < edcaQueue.aifsn * SLOTLENGTH_11P + SIFS_11P) {
				//we didnt even make it one DIFS :(
				info.append(" No DIFS");
			}
			else {
				//decrease the backoff by one because we made it longer than one DIFS
				edcaQueue.currentBackoff -= 1;

				//check how many slots we waited after the first DIFS
				int64_t passedSlots = (int64_t)((passedTime - SimTime(edcaQueue.aifsn * SLOTLENGTH_11P + SIFS_11P)) / SLOTLENGTH_11P);

				DBG_MAC << "Passed slots after DIFS: " << passedSlots << std::endl;


				if (edcaQueue.queue.size() == 0) {
					//this can be below 0 because of post transmit backoff -> backoff on empty queues will not generate macevents,
					//we dont want to generate a txOP for empty queues
					edcaQueue.currentBackoff -= std::min(edcaQueue.currentBackoff,passedSlots);
					info.append(" PostCommit Over");
				}
				else {
					edcaQueue.currentBackoff -= passedSlots;
					if (edcaQueue.currentBackoff <= -1) {
						if (generateTxOp) {
							edcaQueue.txOP = true; info.append(" TXOP");
						}
						//else: this packet couldnt be sent because there was too little time. we could have generated a txop, but the channel switched
						edcaQueue.currentBackoff = 0;
					}

				}
			}
			DBG_MAC << "Updating backoff for Queue " << accessCategory << ": " << oldBackoff << " -> " << edcaQueue.currentBackoff << info <<std::endl;
		}
	}
}
void Mac1609_4::EDCA::backoff(t_access_category ac) {
	myQueues[ac].currentBackoff = OWNER intuniform(0,myQueues[ac].cwCur);
	statsSlotsBackoff += myQueues[ac].currentBackoff;
	statsNumBackoff++;
	DBG_MAC << "Going into Backoff because channel was busy when new packet arrived from upperLayer" << std::endl;
}

void Mac1609_4::EDCA::postTransmit(t_access_category ac, WaveShortMessage* wsm, bool useAcks) {
	bool holBlocking = (wsm->getRecipientAddress() != -1) && useAcks;
	if (holBlocking) {
		//mac->waitUntilAckRXorTimeout = true; // set in handleselfmsg()
		// Head of line blocking, wait until ack timeout
		myQueues[ac].waitForAck = true;
		myQueues[ac].waitOnUnicastID = wsm->getTreeId();
		((Mac1609_4*)owner)->phy11p->notifyMacAboutRxStart(true);
	} else {
		myQueues[ac].waitForAck = false;
		delete myQueues[ac].queue.front();
		myQueues[ac].queue.pop();
		myQueues[ac].cwCur = myQueues[ac].cwMin;
		//post transmit backoff
		myQueues[ac].currentBackoff = OWNER intuniform(0,myQueues[ac].cwCur);
		statsSlotsBackoff += myQueues[ac].currentBackoff;
		statsNumBackoff++;
		DBG_MAC << "Queue " << ac << " will go into post-transmit backoff for " << myQueues[ac].currentBackoff << " slots" << std::endl;
	}
}

Mac1609_4::EDCA::EDCA(cSimpleModule *owner, t_channel channelType, int maxQueueLength)
	: owner(owner),
	  maxQueueSize(maxQueueLength),
	  channelType(channelType),
	  statsNumInternalContention(0),
	  statsNumBackoff(0),
	  statsSlotsBackoff(0) {}

Mac1609_4::EDCA::~EDCA() {
	for (auto &q : myQueues) {
		auto& ackTimeout = q.second.ackTimeOut;
		if (ackTimeout) {
			owner->cancelAndDelete(ackTimeout);
			ackTimeout = nullptr;
		}
	}
}

void Mac1609_4::EDCA::revokeTxOPs() {
	for (auto&& p : myQueues) {
		auto &edcaQueue = p.second;
		if (edcaQueue.txOP == true) {
			edcaQueue.txOP = false;
			edcaQueue.currentBackoff = 0;
		}
	}
}

void Mac1609_4::channelBusySelf(bool generateTxOp) {

	//the channel turned busy because we're sending. we don't want our queues to go into backoff
	//internal contention is already handled in initiateTransmission

	if (!idleChannel) return;
	idleChannel = false;
	DBG_MAC << "Channel turned busy: Switch or Self-Send" << std::endl;

	lastBusy = simTime();

	//channel turned busy
	if (nextMacEvent->isScheduled() == true) {
		cancelEvent(nextMacEvent);
	}
	else {
		//the edca subsystem was not doing anything anyway.
	}
	myEDCA[activeChannel]->stopContent(false, generateTxOp);

	emit(sigChannelBusy, true);
}

void Mac1609_4::channelBusy() {

	if (!idleChannel) return;

	//the channel turned busy because someone else is sending
	idleChannel = false;
	DBG_MAC << "Channel turned busy: External sender" << std::endl;
	lastBusy = simTime();

	//channel turned busy
	if (nextMacEvent->isScheduled() == true) {
		cancelEvent(nextMacEvent);
	}
	else {
		//the edca subsystem was not doing anything anyway.
	}
	myEDCA[activeChannel]->stopContent(true,false);

	emit(sigChannelBusy, true);
}

void Mac1609_4::channelIdle(bool afterSwitch) {

	DBG_MAC << "Channel turned idle: Switch: " << afterSwitch << std::endl;
	if (waitUntilAckRXorTimeout) {
		return;
	}

	if (nextMacEvent->isScheduled() == true) {
		//this rare case can happen when another node's time has such a big offset that the node sent a packet although we already changed the channel
		//the workaround is not trivial and requires a lot of changes to the phy and decider
		return;
		//throw cRuntimeError("channel turned idle but contention timer was scheduled!");
	}

	idleChannel = true;

	simtime_t delay = 0;

	//account for 1609.4 guards
	if (afterSwitch) {
		//	delay = GUARD_INTERVAL_11P;
	}
	if (useSCH) {
		delay += timeLeftTillGuardOver();
	}

	//channel turned idle! lets start contention!
	lastIdle = delay + simTime();
	statsTotalBusyTime += simTime() - lastBusy;

	//get next Event from current EDCA subsystem
	simtime_t nextEvent = myEDCA[activeChannel]->startContent(lastIdle,guardActive());
	if (nextEvent != -1) {
		if ((!useSCH) || (nextEvent < nextChannelSwitch->getArrivalTime())) {
			scheduleAt(nextEvent,nextMacEvent);
			DBG_MAC << "next Event is at " << nextMacEvent->getArrivalTime().raw() << std::endl;
		}
		else {
			DBG_MAC << "Too little time in this interval. will not schedule macEvent" << std::endl;
			statsNumTooLittleTime++;
			myEDCA[activeChannel]->revokeTxOPs();
		}
	}
	else {
		DBG_MAC << "I don't have any new events in this EDCA sub system" << std::endl;
	}

	emit(sigChannelBusy, false);

}

void Mac1609_4::setParametersForBitrate(uint64_t bitrate) {
	for (unsigned int i = 0; i < NUM_BITRATES_80211P; i++) {
		if (bitrate == BITRATES_80211P[i]) {
			n_dbps = N_DBPS_80211P[i];
			return;
		}
	}
	throw cRuntimeError("Chosen Bitrate is not valid for 802.11p: Valid rates are: 3Mbps, 4.5Mbps, 6Mbps, 9Mbps, 12Mbps, 18Mbps, 24Mbps and 27Mbps. Please adjust your omnetpp.ini file accordingly.");
}

bool Mac1609_4::isChannelSwitchingActive() {
    return useSCH;
}

simtime_t Mac1609_4::getSwitchingInterval() {
    return SWITCHING_INTERVAL_11P;
}

bool Mac1609_4::isCurrentChannelCCH() {
    return (activeChannel ==  type_CCH);
}

simtime_t Mac1609_4::getFrameDuration(int payloadLengthBits, enum PHY_MCS mcs) const {
    simtime_t duration;
    if (mcs == MCS_DEFAULT) {
        // calculate frame duration according to Equation (17-29) of the IEEE 802.11-2007 standard
        duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION + T_SYM_80211P * ceil( (16 + payloadLengthBits + 6)/(n_dbps) );
    }
    else {
        uint32_t ndbps = getNDBPS(mcs);
        duration = PHY_HDR_PREAMBLE_DURATION + PHY_HDR_PLCPSIGNAL_DURATION + T_SYM_80211P * ceil( (16 + payloadLengthBits + 6)/(ndbps) );
    }

	return duration;
}

// Unicast
void Mac1609_4::sendAck(int recpAddress, unsigned long wsmId) {
	ASSERT(useAcks);
	// 802.11-2012 9.3.2.8
	// send an ACK after SIFS without regard of busy/ idle state of channel
	ignoreChannelState = true;
	channelBusySelf(true);

	// send the packet
	auto* mac = new Mac80211Ack("ACK");
	mac->setDestAddr(recpAddress);
	mac->setSrcAddr(myMacAddress);
	mac->setMessageId(wsmId);
	mac->setBitLength(ackLength);

	enum PHY_MCS mcs = MCS_DEFAULT;
	uint64_t datarate = getOfdmDatarate(mcs, BW_OFDM_10_MHZ);

	simtime_t sendingDuration = RADIODELAY_11P + getFrameDuration(mac->getBitLength(), mcs);
	DBG_MAC << "Ack sending duration will be " << sendingDuration << std::endl;

	// TODO: check ack procedure when channel switching is allowed
	// double freq = (activeChannel == type_CCH) ? frequency[Channels::CCH] : frequency[mySCH];
	double freq = frequency[Channels::CCH];

	DBG_MAC << "Sending an ack. Frequency " << freq << " at time : " << simTime() + SIFS_11P << std::endl;
	sendFrame(mac, SIFS_11P, freq, datarate, txPower);
	scheduleAt(simTime() + SIFS_11P, stopIgnoreChannelStateMsg);
}

void Mac1609_4::handleUnicast(unique_ptr<WaveShortMessage> wsm) {
	if (useAcks) {
		sendAck(wsm->getSenderAddress(), wsm->getTreeId());
	}

	if (handledUnicastToApp.find(wsm->getTreeId()) == handledUnicastToApp.end()) {
		handledUnicastToApp.insert(wsm->getTreeId());
		DBG_MAC << "Received a data packet addressed to me." << std::endl;
		statsReceivedPackets++;
		sendUp(wsm.release());
	}
}

void Mac1609_4::handleAck(const Mac80211Ack* ack) {
	ASSERT2(rxStartIndication, "Not expecting ack");
	phy11p->notifyMacAboutRxStart(false);
	rxStartIndication = false;

	t_channel chan = type_CCH;
	bool queueUnblocked = false;
	for (auto&& p : myEDCA[chan]->myQueues) {
		auto &accessCategory = p.first;
		auto &edcaQueue = p.second;
		if (edcaQueue.queue.size() > 0 && edcaQueue.waitForAck && (edcaQueue.waitOnUnicastID == ack->getMessageId())) {
			WaveShortMessage* wsm = edcaQueue.queue.front();
			edcaQueue.queue.pop();
			delete wsm;
			myEDCA[chan]->myQueues[accessCategory].cwCur = myEDCA[chan]->myQueues[accessCategory].cwMin;
			myEDCA[chan]->backoff(accessCategory);
			edcaQueue.ssrc = 0;
			edcaQueue.slrc = 0;
			edcaQueue.waitForAck = false;
			edcaQueue.waitOnUnicastID = -1;
			if (myEDCA[chan]->myQueues[accessCategory].ackTimeOut->isScheduled()) {
				cancelEvent(myEDCA[chan]->myQueues[accessCategory].ackTimeOut);
			}
			queueUnblocked = true;
		}
	}
	if (!queueUnblocked) {
		throw cRuntimeError("Could not find WSM in EDCA queues with WSM ID received in ACK");
	} else {
		waitUntilAckRXorTimeout = false;
	}
}

void Mac1609_4::handleAckTimeOut(AckTimeOutMessage* ackTimeOutMsg) {
	if (rxStartIndication) {
		// Rx is already in process. Wait for it to complete.
		// In case it is not an ack, we will retransmit
		// This assigning might be redundant as it was set already in handleSelfMsg but no harm in reassigning here.
		lastAC = (t_access_category)(ackTimeOutMsg->getKind());
		return;
	}
	// We did not start receiving any packet.
	// stop receiving notification for rx start as we will retransmit
	phy11p->notifyMacAboutRxStart(false);
	// back off and try retransmission again
	handleRetransmit((t_access_category)(ackTimeOutMsg->getKind()));
	// Phy was requested not to send channel idle status on TX_OVER
	// So request the channel status now. For the case when we receive ACK, decider updates channel status itself after ACK RX
	phy11p->requestChannelStatusIfIdle();
}

void Mac1609_4::handleRetransmit(t_access_category ac) {
	// cancel the acktime out
	if (myEDCA[type_CCH]->myQueues[ac].ackTimeOut->isScheduled()) {
		// This case is possible if we received PHY_RX_END_WITH_SUCCESS or FAILURE even before ack timeout
		cancelEvent(myEDCA[type_CCH]->myQueues[ac].ackTimeOut);
	}
	if(myEDCA[type_CCH]->myQueues[ac].queue.size() == 0) {
		throw cRuntimeError("Trying retransmission on empty queue...");
	}
	WaveShortMessage* appPkt = myEDCA[type_CCH]->myQueues[ac].queue.front();
	bool contend = false;
	bool retriesExceeded = false;
	// page 879 of IEEE 802.11-2012
	if (appPkt->getBitLength() <= dot11RTSThreshold) {
		myEDCA[type_CCH]->myQueues[ac].ssrc++;
		if (myEDCA[type_CCH]->myQueues[ac].ssrc <= dot11ShortRetryLimit) {
			retriesExceeded = false;
		} else {
			retriesExceeded = true;
		}
	} else {
		myEDCA[type_CCH]->myQueues[ac].slrc++;
		if (myEDCA[type_CCH]->myQueues[ac].slrc <= dot11LongRetryLimit) {
			retriesExceeded = false;
		} else {
			retriesExceeded = true;
		}
	}
	if (!retriesExceeded) {
		// try again!
		myEDCA[type_CCH]->myQueues[ac].cwCur = std::min(myEDCA[type_CCH]->myQueues[ac].cwMax, (myEDCA[type_CCH]->myQueues[ac].cwCur*2)+1);
		myEDCA[type_CCH]->backoff(ac);
		contend = true;
		// no need to reset wait on id here as we are still retransmitting same packet
		myEDCA[type_CCH]->myQueues[ac].waitForAck = false;
	} else {
		// enough tries!
		myEDCA[type_CCH]->myQueues[ac].queue.pop();
		if (myEDCA[type_CCH]->myQueues[ac].queue.size() > 0) {
			// start contention only if there are more packets in the queue
			contend = true;
		}
		delete appPkt;
		myEDCA[type_CCH]->myQueues[ac].cwCur = myEDCA[type_CCH]->myQueues[ac].cwMin;
		myEDCA[type_CCH]->backoff(ac);
		myEDCA[type_CCH]->myQueues[ac].waitForAck = false;
		myEDCA[type_CCH]->myQueues[ac].waitOnUnicastID = -1;
		myEDCA[type_CCH]->myQueues[ac].ssrc = 0;
		myEDCA[type_CCH]->myQueues[ac].slrc = 0;
	}
	waitUntilAckRXorTimeout = false;
	if (contend && idleChannel && !ignoreChannelState) {
		// reevaluate times -- if channel is not idle, then contention would start automatically
		cancelEvent(nextMacEvent);
		simtime_t nextEvent = myEDCA[type_CCH]->startContent(lastIdle, guardActive());
		scheduleAt(nextEvent, nextMacEvent);
	}
}

Mac1609_4::EDCA::EDCAQueue::EDCAQueue(int aifsn,int cwMin, int cwMax, t_access_category ac)
	: aifsn(aifsn),
	  cwMin(cwMin),
	  cwMax(cwMax),
	  cwCur(cwMin),
	  currentBackoff(0),
	  txOP(false),
	  ssrc(0),
	  slrc(0),
	  waitForAck(false),
	  waitOnUnicastID(-1),
	  ackTimeOut(new AckTimeOutMessage("AckTimeOut")) {
	ackTimeOut->setKind(ac);
}

Mac1609_4::EDCA::EDCAQueue::~EDCAQueue() {
	while (!queue.empty()) {
		delete queue.front();
		queue.pop();
	}
	// ackTimeOut needs to be deleted in EDCA
}
