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
#include <BaseModule.h>
#include <BaseUtility.h>
#include "Packet.h"
#include "UWBIRPacket.h"

/**
 * TODO - Generated class
 */
class phyPER : public  BaseModule
{
  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg) { };

    int catPacket, catUWBIRPacket;
    long nbSyncAttempts, nbSyncSuccesses, nbRx;

    Packet packet;
    UWBIRPacket uwbirpacket;

    cOutVector maiPER;

  public:
	/** @brief Called by the Blackboard whenever a change occurs we're interested in */
	virtual void receiveBBItem(int category, const BBItem * details, int scopeModuleId);

};

#endif
