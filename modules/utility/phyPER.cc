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

Define_Module(phyPER);

void phyPER::initialize(int stage)
{
	BaseModule::initialize(stage);
	if(stage == 0) {
		catPacket = utility->subscribe(this, &packet, -1);
		catUWBIRPacket = utility->subscribe(this, &uwbirpacket, -1);
		maiPER.setName("maiPER");
	}
}


void phyPER::receiveBBItem(int category, const BBItem * details, int scopeModuleId) {
    if(category == catPacket) {
    	packet = *(static_cast<const Packet*>(details));
    	nbRx = packet.getNbPacketsReceived();
    } else if(category == catUWBIRPacket) {
    	uwbirpacket = *(static_cast<const UWBIRPacket*>(details));
    	nbSyncAttempts = uwbirpacket.getNbSyncAttempts();
    	nbSyncSuccesses = uwbirpacket.getNbSyncSuccesses();
    }
    if(nbSyncAttempts > 0) {
      maiPER.record( static_cast<double>(nbRx) /nbSyncAttempts );
    }

}
