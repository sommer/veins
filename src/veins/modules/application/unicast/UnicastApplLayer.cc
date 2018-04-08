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

#include "veins/modules/application/unicast/UnicastApplLayer.h"

const simsignalwrap_t UnicastApplLayer::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

Define_Module(UnicastApplLayer);

using namespace std;

void UnicastApplLayer::initialize(int stage) {
	BaseApplLayer::initialize(stage);

	if (stage==0) {
		myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(
		            getParentModule());
		assert(myMac);

		debug = par("debug").boolValue();
		myId = getParentModule()->getIndex();

		headerLength = par("headerLength").longValue();
		double maxOffset = par("maxOffset").doubleValue();

		sendUnicastEvt = new cMessage("unicast", SEND_UNICAST_EVT);

		sendUnicast = par("sendUnicast").boolValue();
		unicastLengthBits = par("unicastLengthBits").longValue();

		individualOffset = dblrand() * maxOffset;
		findHost()->subscribe(mobilityStateChangedSignal, this);

		double unicastOffset = dblrand() * (par("unicastInterval").doubleValue()/2);
		unicastOffset = unicastOffset + floor(unicastOffset/0.050)*0.050;
		if (sendUnicast) {
		    unicastStartTime = par("unicastStartTime").doubleValue();
		    simtime_t unicastStart = max(unicastStartTime, simTime());
		    scheduleAt(unicastStart + unicastOffset, sendUnicastEvt);
		    std::cout << "Starting to send unicasts at: " << unicastStart + unicastOffset << std::endl;
		}

		periodicMeasurementEvt = new cMessage("PeriodicMeasurementUnicastApp", PERIODIC_MEASUREMENT_EVT);
		periodicMeasurementStart = par("periodicMeasurementStartTime").doubleValue();
		periodicMeasurementInterval = par("periodicMeasurementInterval").doubleValue();

		totalBitsReceived = 0;
		measureCount = 0;
		statsBitsRecvdLastMmtPeriod = 0;
		sigGoodput = registerSignal("sigGoodput");

		scheduleAt(max(simTime(), periodicMeasurementStart), periodicMeasurementEvt);
	}
}

WaveShortMessage* UnicastApplLayer::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
	WaveShortMessage* wsm =	new WaveShortMessage(name.c_str());
	wsm->addBitLength(headerLength);
	wsm->addBitLength(lengthBits);

	wsm->setChannelNumber(Channels::CCH);
	wsm->setPsid(0);
	wsm->setUserPriority(priority);
	wsm->setWsmVersion(1);
	wsm->setTimestamp(simTime());
	wsm->setSenderAddress(myId);
	wsm->setRecipientAddress(rcvId);
	wsm->setSerial(serial);
	wsm->setUniqueId(getSimulation()->getUniqueNumber());

	if (sendUnicast && name == "unicast") {
		wsm->setIsUnicast(true);
	}
	return wsm;
}

void UnicastApplLayer::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) {
	Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate(obj);
	}
}

void UnicastApplLayer::handlePositionUpdate(cObject* obj) {
	ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
	curPosition = mobility->getCurrentPosition();
}

void UnicastApplLayer::handleLowerMsg(cMessage* msg) {

	WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
	ASSERT(wsm);

	if (std::string(wsm->getName()) == "unicast") {
		onUnicast(wsm);
	} else {
		DBG << "unknown message (" << wsm->getName() << ")  received\n";
	}
	delete(msg);
}

void UnicastApplLayer::handleSelfMsg(cMessage* msg) {
	switch (msg->getKind()) {
		case SEND_UNICAST_EVT: {
			sendUnicastPacket();
			break;
		}
		case PERIODIC_MEASUREMENT_EVT:
			recordGoodput();
			scheduleAt(simTime() + periodicMeasurementInterval, periodicMeasurementEvt);
			break;
		default: {
			if (msg)
				DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
			break;
		}
	}
}

void UnicastApplLayer::sendUnicastPacket() {
    sendWSM(prepareWSM("unicast", unicastLengthBits, type_CCH, 0, 1, -1));
    unicastSentCount++;
    scheduleAt(simTime() + par("unicastInterval").doubleValue(), sendUnicastEvt);
}

void UnicastApplLayer::sendWSM(WaveShortMessage* wsm) {
	sendDelayedDown(wsm,individualOffset);
}

void UnicastApplLayer::onUnicast(WaveShortMessage* wsm) {
    //std::cout << "Application at node: " << myId << " received a unicast with id: " << wsm->getUniqueId() << " from " << wsm->getSenderAddress() << std::endl;
    statsBitsRecvdLastMmtPeriod += wsm->getBitLength() - headerLength;
}

void UnicastApplLayer::finish() {
	findHost()->unsubscribe(mobilityStateChangedSignal, this);
	recordScalar("UnicastLengthBits", unicastLengthBits);
	recordScalar("HeaderLengthBits", headerLength);
}

UnicastApplLayer::~UnicastApplLayer() {}

void UnicastApplLayer::recordGoodput() {
    emit(sigGoodput, statsBitsRecvdLastMmtPeriod);
    totalBitsReceived += ((double)statsBitsRecvdLastMmtPeriod)/(1024*1024);
    measureCount++;
    cout << simTime() << ": Mbits received in last second: " << ((double)statsBitsRecvdLastMmtPeriod)/(1024*1024) << " and avg: " << totalBitsReceived/measureCount << " Mbits/sec" << endl;
    statsBitsRecvdLastMmtPeriod = 0;
}
