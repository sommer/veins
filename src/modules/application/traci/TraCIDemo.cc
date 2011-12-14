//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include "application/traci/TraCIDemo.h"

#include "NetwControlInfo.h"
#include "SimpleAddress.h"
#include "ApplPkt_m.h"

Define_Module(TraCIDemo);

const simsignalwrap_t TraCIDemo::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void TraCIDemo::initialize(int stage) {
	BaseApplLayer::initialize(stage);
	if (stage == 0) {
		debug = par("debug");

		traci = TraCIMobilityAccess().get(getParentModule());
		findHost()->subscribe(mobilityStateChangedSignal, this);

		sentMessage = false;
	}
}

void TraCIDemo::handleSelfMsg(cMessage *msg) {
}

void TraCIDemo::handleLowerMsg(cMessage* msg) {
	if (!sentMessage) sendMessage();
	delete msg;
}

void TraCIDemo::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) {
	Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate();
	}
}

void TraCIDemo::sendMessage() {
	sentMessage = true;

	ApplPkt *pkt = new ApplPkt("BROADCAST_MESSAGE", 0);
	pkt->setDestAddr(-1);
	pkt->setSrcAddr(myApplAddr());
	pkt->setBitLength(headerLength);

	NetwControlInfo::setControlInfo(pkt, LAddress::L3BROADCAST );

	sendDown(pkt);
}

void TraCIDemo::handlePositionUpdate() {
	if (traci->getPositionAt(simTime()).x < 7350) {
		if (!sentMessage) sendMessage();
	}
}

