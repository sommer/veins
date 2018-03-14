//
// Copyright (C) 2015 Dominik Buse <dbuse@mail.uni-paderborn.de>
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

#include "veins/modules/application/traci/TraCICAMSender.h"
#include "veins/modules/messages/CooperativeAwarenessMessage_m.h"

using Veins::TraCIMobilityAccess;

Define_Module(TraCICAMSender);

TraCICAMSender::~TraCICAMSender() {
	cancelAndDelete(camTimer);
}

void TraCICAMSender::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	if (stage == 0) {
		mobility = TraCIMobilityAccess().get(getParentModule());
		traci = mobility->getCommandInterface();
		traciVehicle = mobility->getVehicleCommandInterface();
		camInterval = par("camInterval").doubleValue();
		camTimer = new cMessage("sendCAM");
		scheduleAt(simTime() + uniform(0, camInterval), camTimer);
	}
}

void TraCICAMSender::handleSelfMsg(cMessage* msg) {
	if(msg == camTimer) {
		sendCAM();
		scheduleAt(simTime() + camInterval, camTimer);
	} else {
		BaseWaveApplLayer::handleSelfMsg(msg);
	}
}

void TraCICAMSender::sendCAM() {
	CooperativeAwarenessMessage* cam = new CooperativeAwarenessMessage("data");
	cam->addBitLength(headerLength);
	cam->addBitLength(dataLengthBits);
	cam->setChannelNumber(Channels::CCH); // always use Control Channel for now
	cam->setPsid(0);
	cam->setUserPriority(dataUserPriority);
	cam->setWsmVersion(1);
	cam->setWsmLength(5 * 64); // id, xpos, ypos, speed, angle; each 64 bit
	cam->setTimestamp(simTime());
	cam->setSenderAddress(myId);
	cam->setRecipientAddress(-1);
	cam->setSenderPos(mobility->getCurrentPosition());
	cam->setSerial(2);
	cam->setVehicleId(mobility->getExternalId().c_str());
	cam->setAngle(mobility->getAngleRad());
	cam->setSpeed(mobility->getSpeed());
	cam->setCurrentEdge(mobility->getRoadId().c_str());
	cam->setCurrentLandNr(mobility->getLaneNr());
	cam->setCurrentLanePosition(mobility->getLanePosition());
	cam->setPsc("CAM");
	sendDown(cam);
}
