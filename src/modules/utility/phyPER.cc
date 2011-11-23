//
// (c) 2009 CSEM SA, Neuchâtel, Switzerland.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "phyPER.h"

#include "BaseLayer.h"
#include "DeciderUWBIRED.h"

Define_Module(phyPER);

void phyPER::initialize(int stage)
{
	BaseModule::initialize(stage);
	if(stage == 0) {
		findHost()->subscribe(BaseLayer::catPacketSignal, this);
		findHost()->subscribe(DeciderUWBIRED::catUWBIRPacketSignal, this);
		maiPER.setName("maiPER");
		maiPERnoRS.setName("maiPERnoRS");
		nbSyncAttempts = 0;
		nbSyncSuccesses = 0;
		nbRx = 0;
		nbRxnoRS = 0;
	}
}


void phyPER::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) {
    if(signalID == BaseLayer::catPacketSignal) {
    	packet = *(static_cast<const Packet*>(obj));
    	nbRx = static_cast<long>(packet.getNbPacketsReceived());
    	nbRxnoRS = static_cast<long>(packet.getNbPacketsReceivedNoRS());
    } else if(signalID == DeciderUWBIRED::catUWBIRPacketSignal) {
    	uwbirpacket = *(static_cast<const UWBIRPacket*>(obj));
    	nbSyncAttempts = static_cast<long>(uwbirpacket.getNbSyncAttempts());
    	nbSyncSuccesses = uwbirpacket.getNbSyncSuccesses();
    }
    if(nbSyncAttempts > 0) {
      maiPER.record( static_cast<double>(nbRx) / nbSyncAttempts );
      maiPERnoRS.record( static_cast<double>(nbRxnoRS) / nbSyncAttempts );
    }
}
