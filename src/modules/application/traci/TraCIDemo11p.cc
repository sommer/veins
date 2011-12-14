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

#include "application/traci/TraCIDemo11p.h"

Define_Module(TraCIDemo11p);

const simsignalwrap_t TraCIDemo11p::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void TraCIDemo11p::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	if (stage == 0) {
		traci = TraCIMobilityAccess().get(getParentModule());
		findHost()->subscribe(mobilityStateChangedSignal, this);

		sentMessage = false;
		lastDroveAt = simTime();
	}
}

void TraCIDemo11p::onBeacon(WaveShortMessage* wsm) {
}

void TraCIDemo11p::onData(WaveShortMessage* wsm) {
	findHost()->getDisplayString().updateWith("r=16,green");
	if (!sentMessage) sendMessage();
}

void TraCIDemo11p::sendMessage() {
	sentMessage = true;

	t_channel channel = dataOnSch ? type_SCH : type_CCH;
	sendWSM(prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2));
}


void TraCIDemo11p::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
	Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate();
	}
}

void TraCIDemo11p::handlePositionUpdate() {
	// stopped for for at least 10s?
	if (traci->getSpeed() < 1) {
		if (simTime() - lastDroveAt >= 10) {
			findHost()->getDisplayString().updateWith("r=16,red");
			if (!sentMessage) sendMessage();
		}
	}
	else {
		lastDroveAt = simTime();
	}
}

