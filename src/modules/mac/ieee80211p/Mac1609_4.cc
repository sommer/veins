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

#include "Mac1609_4.h"

Define_Module(Mac1609_4);

void Mac1609_4::initialize(int stage) {
	BaseLayer::initialize(stage);
	if (stage == 0) {

		phy11p = FindModule<Mac80211pToPhy11pInterface*>::findSubModule(
		             getParentModule()->getParentModule());
		assert(phy11p);

		frequency.insert(std::pair<int, double>(Channels::CRIT_SOL, 5.86e9));
		frequency.insert(std::pair<int, double>(Channels::SCH1, 5.87e9));
		frequency.insert(std::pair<int, double>(Channels::SCH2, 5.88e9));
		frequency.insert(std::pair<int, double>(Channels::CCH, 5.89e9));
		frequency.insert(std::pair<int, double>(Channels::SCH3, 5.90e9));
		frequency.insert(std::pair<int, double>(Channels::SCH4, 5.91e9));
		frequency.insert(std::pair<int, double>(Channels::HPPS, 5.92e9));

		switch (par("serviceChannel").longValue()) {
			case 1: mySCH = Channels::SCH1; break;
			case 2: mySCH = Channels::SCH2; break;
			case 3: mySCH = Channels::SCH3; break;
			case 4: mySCH = Channels::SCH4; break;
			default: opp_error("Service Channel must be between 1 and 4"); break;
		}

		if (par("serviceChannel").longValue() == -1) {
			mySCH = (getParentModule()->getParentModule()->getIndex() % 2) ?
			        Channels::SCH1 : Channels::SCH2;
		}

		headerLength = par("headerLength");

		channel_switch = new cMessage("Channel Switch");

		//introduce a little asynchronization between radios, but no more than 2 milliseconds
		uint64_t currenTime = simTime().raw();
		uint64_t switchingTime = (uint64_t)(SWITCHING_INTERVAL_11P
		                                    * simTime().getScale());
		double timeToNextSwitch = (double)(switchingTime
		                                   - (currenTime % switchingTime)) / simTime().getScale();

		double individualOffset = std::min(3e-3, truncnormal(0, 1e-3, 0));

		scheduleAt(simTime() + individualOffset + timeToNextSwitch,
		           channel_switch);

		if ((currenTime / switchingTime) % 2 == 0) {
			setCsState(CCH);
		}
		else {
			setCsState(SCH);
		}
	}
}

void Mac1609_4::handleSelfMsg(cMessage* msg) {
	if (msg == channel_switch) {
		switch (csState) {
			case CCH:
				setCsState(SCH);
				DBG << "Switching to SCH" << std::endl;
				phy11p->changeListeningFrequency(frequency[mySCH]);
				if (!schQueue.empty()) {
					DBG << "SCH Queue is not empty. Size : " << schQueue.size()
					    << ". Sending first packet from Queue" << std::endl;
					attachAndSend(schQueue.front(),SCH, type_SCH, frequency[mySCH]);
				}
				break;
			case SCH:
				setCsState(CCH);
				DBG << "Switching to CCH" << std::endl;
				phy11p->changeListeningFrequency(frequency[Channels::CCH]);
				if (!cchQueue.empty()) {
					DBG << "CCH Queue is not empty. Size : " << cchQueue.size()
					    << ". Sending first packet from Queue" << std::endl;
					attachAndSend(cchQueue.front(),CCH, type_CCH, frequency[Channels::CCH]);
				}
				break;
		}
		//schedule next channel switch in 50ms
		scheduleAt(simTime() + SWITCHING_INTERVAL_11P, channel_switch);
	}
}

void Mac1609_4::attachAndSend(cMessage* msg, ChannelSelectorState channel, t_channel channelType, double frequency) {
	lastPacketCameFrom = channel;

	int priority = (dynamic_cast<WaveShortMessage*>(msg))->getPriority();
	ASSERT(priority <= 3 && priority >= 0);

	Mac1609_4To80211pControlInfo* addInfo = new Mac1609_4To80211pControlInfo(-1, priority, frequency, channelType);

	Mac80211Pkt* pkt = new Mac80211Pkt("MacPkt including WSA");
	pkt->setControlInfo(addInfo);
	pkt->addBitLength(headerLength);
	pkt->encapsulate(dynamic_cast<WaveShortMessage*>(msg->dup()));
	DBG << "Mac Sending down a SCH Packet with priority :" << priority << std::endl;
	sendDown(pkt);
}

void Mac1609_4::handleUpperControl(cMessage* msg) {
	assert(false);
}

void Mac1609_4::handleUpperMsg(cMessage* msg) {

	WaveShortMessage* thisMsg;
	if ((thisMsg = dynamic_cast<WaveShortMessage*>(msg)) != NULL) {
		DBG << "Received a message from upper layer for channel "
		    << thisMsg->getChannelNumber() << " Access Category (Priority):  "
		    << thisMsg->getPriority() << std::endl;

		//rewrite SCH channel to actual SCH the Mac1609_4 is set to
		if (thisMsg->getChannelNumber() == Channels::SCH1) {
			thisMsg->setChannelNumber(mySCH);
		}

		if (thisMsg->getChannelNumber() == Channels::CCH) {
			if ((csState == CCH) && (cchQueue.empty())) {
				//Send this CCH packet right away
				cchQueue.push(msg);
				attachAndSend(msg,CCH, type_CCH, frequency[Channels::CCH]);

			}
			else {
				//Queue this packet because CCH is not active or Queue is not empty
				cchQueue.push(msg);
			}
		}
		else {

			if ((csState == SCH) && (schQueue.empty())) {
				//Send this SCH packet right away
				schQueue.push(msg);
				attachAndSend(msg,SCH, type_SCH, frequency[mySCH]);
			}
			else {
				//Queue this packet because SCH is not active or Queue is not empty
				schQueue.push(msg);
			}
		}
	}
}

void Mac1609_4::handleLowerMsg(cMessage* msg) {
	sendUp(msg);
}

void Mac1609_4::handleLowerControl(cMessage* msg) {
	if (msg->getKind() == MacToPhyInterface::TX_OVER || msg->getKind() == Mac80211pToMac1609_4Interface::PACKET_DROPPED) {
		//if a packet was dropped, we can't do anything about, so we treat it the same as if it was sent
		//remove last packet from Channel Queue
		switch (lastPacketCameFrom) {
			case CCH:
				delete cchQueue.front();
				cchQueue.pop();
				break;
			case SCH:
				delete schQueue.front();
				schQueue.pop();
				break;
			default:
				DBG << "Error: Last packet came from unknown state!"<< std::endl;
				assert(false);
				break;
		}
		//Send next packet down
		switch (csState) {
			case CCH:
				if (!cchQueue.empty())
					attachAndSend(cchQueue.front(),CCH, type_CCH, frequency[Channels::CCH]);
				break;
			case SCH:
				if (!schQueue.empty())
					attachAndSend(schQueue.front(),SCH, type_SCH, frequency[mySCH]);
				break;
			default:
				assert(false);
				break;
		}
	}
	else if (msg->getKind() == Mac80211pToMac1609_4Interface::INTERVAL_TO_SHORT) {
		// Mac was informed that time was too short to send packet
		// We do not pop a packet from the Queue now. Will be retransmitted automatically on next channel switch
	}
	else {
		DBG << "Invalid control message type (type=NOTHING) : name="
		    << msg->getName() << " modulesrc="
		    << msg->getSenderModule()->getFullPath() << "." << endl;
		assert(false);
	}
	delete msg;
}

void Mac1609_4::setCsState(ChannelSelectorState state) {
	csState = state;
	assert(state == CCH || state == SCH);
}

void Mac1609_4::finish() {
	//clean up queues.
	while (!cchQueue.empty()) {
		delete cchQueue.front();
		cchQueue.pop();
	}

	while (!schQueue.empty()) {
		delete schQueue.front();
		schQueue.pop();
	}

	if (channel_switch->isScheduled())
		cancelAndDelete(channel_switch);
}

/* checks if guard is active */
bool Mac1609_4::guardActive() const {
	if (simTime().dbl() - channel_switch->getSendingTime() <= GUARD_INTERVAL_11P)
		return true;
	return false;
}

/* returns the time until the guard is over */
double Mac1609_4::timeLeftTillGuardOver() const {
	double sTime = simTime().dbl();
	if (sTime - channel_switch->getSendingTime() <= GUARD_INTERVAL_11P) {
		return GUARD_INTERVAL_11P
		       - (sTime - channel_switch->getSendingTime().dbl());
	}
	else
		return 0;
}

/* returns the time left in this channel window */
double Mac1609_4::timeLeft() const {
	return channel_switch->getArrivalTime().dbl() - simTime().dbl();
}

/* Will change the Service Channel on which the mac layer is listening and sending */
void Mac1609_4::changeServiceChannel(int cN) {
	if (cN != Channels::SCH1 && cN != Channels::SCH2 && cN != Channels::SCH3 && cN != Channels::SCH4) {
		opp_error("This Service Channel doesnt exit: %d",cN);
	}

	mySCH = cN;

	if (csState == SCH) {
		//change to new chan immediately if we are in a SCH slot,
		//otherwise it will switch to the new SCH upon next channel switch
		phy11p->changeListeningFrequency(frequency[mySCH]);
	}

}

