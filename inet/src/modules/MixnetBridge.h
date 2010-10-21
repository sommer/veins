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
#include <MixnetWorldUtility.h>

/**
 * @brief INET <-> MiXiM compatibility class. Converts packets between
 * INET network layer and MiXiM NIC.
 *
 * For support of MiXiM NICs in INET hosts connect the MiXiM
 * NIC with the lower gates of the bridge and the INET network layer
 * with the upper gates of this bridge.
 *
 * Does the following things:
 * - registers the NIC connected to its lowerGateOut in INETs Interface
 *   table
 * - dumps control messages sent by the NIC (INET doesn't support them)
 * - forwards packets from NIC to upper layer
 * - converts the Ieee802Ctrl control info (INET) of packets sent from upper
 *   layer to NIC into a NetwToMacControlInfo (MiXiM)
 * - converts INET MAC-addresses to MiXiM MAC-addresses
 *
 * This class expects that the MiXiM NIC uses the NIC modules id as MAC-address
 * to work. Therefore no addressing module implementing MiXiM's
 * "AddressingInterface" should be present in the host!
 *
 * Uses MixnetWorldUtility as INET<->MiXiM MAC-address database.
 *
 * @ingroup mixnet
 *
 * @author Karl Wessel
 */
class MixnetBridge : public cSimpleModule
{
protected:
	/** @brief INET's MAC-address for this bridge's NIC.*/
	MACAddress myINETMacAddr;
	/** @brief MiXiM's MAC-address for this bridge's NIC.*/
	int myMiximMacAddr;

	/** @brief Pointer to MIxNET's world utility module.*/
	MixnetWorldUtility* world;

	/** @brief Pointer to this bridge's NIC module.*/
	cModule* nic;

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
    /**
     * @brief Registers this bridge's NIC with INET's InterfaceTable.
     */
    void registerInterface();

    /**
     * @brief Handles messages from upper layer
     */
    virtual void handleUpperMsg(cMessage *msg);

    /**
     * @brief Handles messages from lower layer
     */
    virtual void handleLowerMsg(cMessage *msg);

    /**
     * @brief Looks for this bridge's module by using the
     * lowerGateOut connection.
     *
     * @return Pointer to this bridge's NIC module or NULL
     */
    cModule* findMyNic();
};

#endif
