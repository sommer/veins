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

#ifndef __PHYPER_H__
#define __PHYPER_H__

#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseModule.h"
#include "Packet.h"
#include "UWBIRPacket.h"

/**
 */
class MIXIM_API phyPER : public  BaseModule
{
  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg) { };

    long        nbSyncAttempts;
    long        nbSyncSuccesses;
    long        nbRx;
    long        nbRxnoRS;

    Packet      packet;
    UWBIRPacket uwbirpacket;

    cOutVector  maiPER;
    cOutVector  maiPERnoRS;

  public:

    phyPER(): packet(100) {}

	/** @brief Called by the signaling mechanism whenever a change occurs we're interested in */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
};

#endif
