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
#include <DeciderResult802154Narrow.h>
#include <PhyToMacControlInfo.h>

Define_Module(CSMA802154);

void CSMA802154::initialize(int stage)
{
	csma::initialize(stage);
}

cPacket *CSMA802154::decapsMsg(MacPkt * macPkt) {

	cPacket * msg = csma::decapsMsg(macPkt);

	PhyToMacControlInfo* cinfo = static_cast<PhyToMacControlInfo*> (macPkt->getControlInfo());
	const DeciderResult802154Narrow* result = static_cast<const DeciderResult802154Narrow*> (cinfo->getDeciderResult());
	double ber = result->getBER();
	//TODO: pass bit error rate with control info to upper layer

	return msg;
}
