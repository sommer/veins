/*
 *  Copyright (C) 2011 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "application/traci/TraCIDemo.h"

#include "NetwControlInfo.h"
#include "SimpleAddress.h"

Define_Module(TraCIDemo);

void TraCIDemo::initialize(int stage) {
	BaseApplLayer::initialize(stage);
	if (stage == 0) {
		debug = par("debug");

		traci = TraCIMobilityAccess().get(getParentModule());
		sentMessage = false;

		Move move;
		catMove = utility->subscribe(this, &move, findHost()->getId());
	}
}

void TraCIDemo::handleSelfMsg(cMessage *msg) {
}

void TraCIDemo::handleLowerMsg(cMessage* msg) {
	if (!sentMessage) sendMessage();
	delete msg;
}

void TraCIDemo::receiveBBItem(int category, const BBItem *details, int scopeModuleId) {
	BaseApplLayer::receiveBBItem(category, details, scopeModuleId);

	Enter_Method_Silent();

	if (category == catMove) {
		handlePositionUpdate();
	}
}

void TraCIDemo::sendMessage() {
	sentMessage = true;

	ApplPkt *pkt = new ApplPkt("BROADCAST_MESSAGE", 0);
	pkt->setDestAddr(-1);
	pkt->setSrcAddr(myApplAddr());
	pkt->setBitLength(headerLength);

	pkt->setControlInfo(new NetwControlInfo(L3BROADCAST));

	sendDown(pkt);
}

void TraCIDemo::handlePositionUpdate() {
	if (traci->getPosition().getX() < 7350) {
		if (!sentMessage) sendMessage();
	}
}

