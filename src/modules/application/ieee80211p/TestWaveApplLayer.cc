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

#include "TestWaveApplLayer.h"

Define_Module(TestWaveApplLayer);

void TestWaveApplLayer::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	receivedBeacons = 0;
	receivedData = 0;
}

void TestWaveApplLayer::onBeacon(WaveShortMessage* wsm) {
	receivedBeacons++;

	DBG << "Received beacon priority  " << wsm->getPriority() << " at " << simTime() << std::endl;
	int senderId = wsm->getSenderAddress();

	if (sendData) {
		t_channel channel = dataOnSch ? type_SCH : type_CCH;
		sendWSM(prepareWSM("data", dataLengthBits, channel, dataPriority, senderId,2));
	}
}

void TestWaveApplLayer::onData(WaveShortMessage* wsm) {

	int recipientId = wsm->getRecipientAddress();

	if (recipientId == myId) {
		DBG  << "Received data priority  " << wsm->getPriority() << " at " << simTime() << std::endl;
		receivedData++;
	}
}

TestWaveApplLayer::~TestWaveApplLayer() {

}


