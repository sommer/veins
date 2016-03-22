/* -*- mode:c++ -*- ********************************************************
 * file:        BaseMacLayer.h
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: basic MAC layer class
 *              subclass to create your own MAC layer
 **************************************************************************/
#ifndef BASE_MAC_LAYER_H
#define BASE_MAC_LAYER_H

#include <omnetpp.h>

#include "veins/base/utils/MiXiMDefs.h"
#include "veins/base/modules/BaseLayer.h"
#include "veins/base/utils/SimpleAddress.h"
#include "veins/base/phyLayer/MappingBase.h"

class BaseConnectionManager;
class MacPkt;
class MacToPhyInterface;
class Signal;

/**
 * @brief A very simple MAC module template which provides de- and
 * encapsulation of messages using the standard addresses. It also
 * provides basic handling of lower layer messages.
 *
 * @ingroup macLayer
 * @ingroup baseModules
 * @author Daniel Willkomm, Karl Wessel
 */
class MIXIM_API BaseMacLayer : public BaseLayer
{
public:
    /** @brief Message kinds used by this layer.*/
    enum BaseMacMessageKinds {
        /** Stores the id on which classes extending BaseMac should
         * continue their own message kinds.*/
        LAST_BASE_MAC_MESSAGE_KIND = 23000,
    };
    /** @brief Control message kinds used by this layer.*/
    enum BaseMacControlKinds {
        /** Indicates the end of a transmission*/
        TX_OVER = 23500,
        /** Tells the netw layer that a packet to be sent has been dropped.*/
        PACKET_DROPPED,
        /** Stores the id on which classes extending BaseMac should
         * continue their own control kinds.*/
        LAST_BASE_MAC_CONTROL_KIND,
    };

protected:

    /** @brief Handler to the physical layer.*/
    MacToPhyInterface* phy;

    /** @brief Pointer to the arp module*/
    //BaseArp* arp;

    /**
     * @brief Length of the MacPkt header
     **/
    int headerLength;

    /**
     * @brief MAC address.
     **/
    LAddress::L2Type myMacAddr;

    /** @brief debug this core module? */
    bool coreDebug;

    /** @brief The length of the phy header (in bits).
     *
     * Since the MAC layer has to create the signal for
     * a transmission it has to know the total length of
     * the packet and therefore needs the length of the
     * phy header.
     */
    int phyHeaderLength;

public:
    //Module_Class_Members( BaseMacLayer, BaseLayer, 0 );
    BaseMacLayer() 
      : BaseLayer()
      , phy(NULL)
      , myMacAddr(LAddress::L2NULL())
    {}
    BaseMacLayer(unsigned stacksize) 
      : BaseLayer(stacksize)
      , phy(NULL)
      , myMacAddr(LAddress::L2NULL())
    {}

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /**
     * @brief Returns the MAC address of this MAC module.
     */
    const LAddress::L2Type& getMACAddress() { return myMacAddr; }

protected:

    /**
     * @brief Registers this bridge's NIC with INET's InterfaceTable.
     */
    virtual void registerInterface();

    /**
     * @brief Handle messages comming from the network layer
     *
     * Here is the place to implement a real medium access functionality.
     *
     * If message arrives from upper layer, make a MAC packet from it
     * (encapsMsg) and send it down (sendDown).
     *
     *  @sa encapsMsg, sendDown
     */
    virtual void handleUpperMsg(cMessage *msg);

    /**
     * If message arrives from lower layer, check whether it is for
     * us. Send it up if yes.
     */
    virtual void handleLowerMsg(cMessage *msg);

    virtual void handleSelfMsg(cMessage* msg){
	error("BaseMacLayer does not handle self messages");
    };
    virtual void handleLowerControl(cMessage* msg);

    virtual void handleUpperControl(cMessage* msg){
	error("BaseMacLayer does not handle control messages from upper layers");
    };


    /** @brief decapsulate the network message from the MacPkt */
    virtual cPacket* decapsMsg(MacPkt*);

    /** @brief Encapsulate the NetwPkt into an MacPkt */
    virtual MacPkt* encapsMsg(cPacket*);

    /**
     * @brief Creates a simple Signal defined over time with the
     * passed parameters.
     *
     * Convenience method to be able to create the appropriate
     * Signal for the MacToPhyControlInfo without needing to care
     * about creating Mappings.
     *
     * NOTE: The created signal's transmission-power is a rectangular function.
     * This method uses MappingUtils::addDiscontinuity to represent the discontinuities
     * at the beginning and end of this rectangular function.
     * Because of this the created mapping which represents the signal's
     * transmission-power is still zero at the exact start and end.
     * Please see the method MappingUtils::addDiscontinuity for the reason.
     */
    virtual Signal* createSimpleSignal(simtime_t_cref start, simtime_t_cref length, double power, double bitrate);

    /**
     * @brief Creates a simple Mapping with a constant curve
     * progression at the passed value.
     *
     * Used by "createSimpleSignal" to create the bitrate mapping.
     */
    Mapping* createConstantMapping(simtime_t_cref start, simtime_t_cref end, Argument::mapped_type_cref value);

    /**
     * @brief Creates a simple Mapping with a constant curve
     * progression at the passed value and discontinuities at the boundaries.
     *
     * Used by "createSimpleSignal" to create the power mapping.
     */
    Mapping* createRectangleMapping(simtime_t_cref start, simtime_t_cref end, Argument::mapped_type_cref value);

    /**
     * @brief Creates a Mapping defined over time and frequency with
     * constant power in a certain frequency band.
     */
    ConstMapping* createSingleFrequencyMapping(simtime_t_cref start, simtime_t_cref end, Argument::mapped_type_cref centerFreq, Argument::mapped_type_cref bandWith, Argument::mapped_type_cref value);

    /**
     * @brief Returns a pointer to this MACs NICs ConnectionManager module.
     * @return pointer to the connection manager module
     */
    BaseConnectionManager* getConnectionManager();

    /**
     * @brief Extracts the MAC address from the "control info" structure (object).
     *
     * Extract the destination MAC address from the "control info" which was prev. set by NetwToMacControlInfo::setControlInfo().
     *
     * @param pCtrlInfo	The "control info" structure (object) prev. set by NetwToMacControlInfo::setControlInfo().
     * @return The MAC address of message receiver.
     */
    virtual const LAddress::L2Type& getUpperDestinationFromControlInfo(const cObject *const pCtrlInfo);

    /**
     * @brief Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg		The message where the "control info" shall be attached.
     * @param pSrcAddr	The MAC address of the message receiver.
     */
    virtual cObject *const setUpControlInfo(cMessage *const pMsg, const LAddress::L2Type& pSrcAddr);
    /**
     * @brief Attaches a "control info" (MacToPhy) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the signal.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg		The message where the "control info" shall be attached.
     * @param pSignal	The signal which should be send.
     */
    virtual cObject *const setDownControlInfo(cMessage *const pMsg, Signal *const pSignal);
};

#endif
