//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "CSMA802154.h"

#include <cassert>

#include "DeciderResult802154Narrow.h"
#include "Decider802154Narrow.h"
#include "PhyToMacControlInfo.h"
#include "MacToNetwControlInfo.h"
#include "MacPkt_m.h"

Define_Module(CSMA802154);


void CSMA802154::initialize(int stage)
{
	csma::initialize(stage);
}

cPacket *CSMA802154::decapsMsg(MacPkt * macPkt) {

	cPacket * msg = csma::decapsMsg(macPkt);

	// get bit error rate
	PhyToMacControlInfo* cinfo = static_cast<PhyToMacControlInfo*> (macPkt->getControlInfo());
	const DeciderResult802154Narrow* result = static_cast<const DeciderResult802154Narrow*> (cinfo->getDeciderResult());
	double ber = result->getBER();
	double rssi = result->getRSSI();

	//get control info attached by base class decapsMsg method
	//and set its rssi and ber
	assert(dynamic_cast<MacToNetwControlInfo*>(msg->getControlInfo()));
	MacToNetwControlInfo* cInfo = static_cast<MacToNetwControlInfo*>(msg->getControlInfo());
	cInfo->setBitErrorRate(ber);
	cInfo->setRSSI(rssi);

	return msg;
}

void CSMA802154::handleLowerControl(cMessage *msg) {
	if (msg->getKind() == Decider802154Narrow::RECEPTION_STARTED) {
		debugEV<< "control message: RECEIVING AirFrame" << endl;
		delete msg;
		return;
	}
	csma::handleLowerControl(msg);
}
