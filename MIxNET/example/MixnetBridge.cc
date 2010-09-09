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

#include "MixnetBridge.h"
#include <InterfaceTableAccess.h>
#include <cassert>

Define_Module(MixnetBridge);

void MixnetBridge::initialize(int stage)
{
	if(stage == 0) {
		upperGateIn  = findGate("upperGateIn");
        upperGateOut = findGate("upperGateOut");
        lowerGateIn  = findGate("lowerGateIn");
        lowerGateOut = findGate("lowerGateOut");
        lowerControlIn  = findGate("lowerControlIn");
        lowerControlOut = findGate("lowerControlOut");
		registerInterface();
	}
}

void MixnetBridge::handleMessage(cMessage *msg)
{
	if(msg->arrivedOn(upperGateIn)) {
		handleUpperMsg(msg);
	} else if(msg->arrivedOn(lowerGateIn)) {
		handleLowerMsg(msg);
	} else {
		assert(msg->arrivedOn(lowerControlIn));
		delete msg;
		ev << "Dumped control message " << msg << " from MAC layer." << endl;
	}
}

void MixnetBridge::handleUpperMsg(cMessage *msg) {

	delete msg;
}

void MixnetBridge::handleLowerMsg(cMessage *msg) {

	delete msg;
}


void MixnetBridge::registerInterface()
{
    InterfaceEntry *e = new InterfaceEntry();

    // interface name: NetworkInterface module's name without special characters ([])
    char *interfaceName = new char[strlen(getParentModule()->getFullName()) + 1];
    char *d = interfaceName;
    for (const char *s = getParentModule()->getFullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    e->setName(interfaceName);
    delete [] interfaceName;

    const char *addrstr = par("address");
    if (!strcmp(addrstr, "auto"))
    {
        // assign automatic address
        myMacAddr = MACAddress::generateAutoAddress();

        // change module parameter from "auto" to concrete address
        par("address").setStringValue(myMacAddr.str().c_str());
    }
    else
    {
        myMacAddr.setAddress(addrstr);
    }
    e->setMACAddress(myMacAddr);

    // generate interface identifier for IPv6
    e->setInterfaceToken(myMacAddr.formInterfaceIdentifier());

    // MTU on 802.11 = ?
    e->setMtu(par("mtu"));            // FIXME

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    // add
    IInterfaceTable *ift = InterfaceTableAccess().get();
    ift->addInterface(e, this);
}
