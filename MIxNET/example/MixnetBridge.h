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

#ifndef __MIXNETBRIDGE_H__
#define __MIXNETBRIDGE_H__

#include <omnetpp.h>
#include <MACAddress.h>
#include <BaseLayer.h>

/**
 * TODO - Generated class
 */
class MixnetBridge : public cSimpleModule
{
protected:
	MACAddress myMacAddr;

	/** @name gate ids*/
    /*@{*/
    int upperGateIn;
    int upperGateOut;
    int lowerGateIn;
    int lowerGateOut;
    int lowerControlIn;
    int lowerControlOut;

    /*@}*/

public:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual int numInitStages() const {return 2;}

protected:
    void registerInterface();

    /** @brief Handle messages from upper layer
     */
    virtual void handleUpperMsg(cMessage *msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage *msg);
};

#endif
