/* -*- mode:c++ -*- ********************************************************
 * file:        AlohaMacLayer.h
 *
 * author:      Marc Loebbers, Yosia Hadisusanto
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
 ***************************************************************************/


#ifndef ALOHAMAC_LAYER_H
#define ALOHAMAC_LAYER_H

#include <list>
#include <ActiveChannel.h>
#include <BasicMacLayer.h>

#include "RadioState.h"
#include "SingleChannelRadio.h"


/**
 * @class AlohaMacLayer
 * @brief MAC module which provides pure Aloha
 *
 * This is an implementation of a pure Aloha (not slottes). The idea
 * of Aloha is to send a packet whenever a packet is ready to be sent.
 *
 * If a packet from the upper layer arrives although there is still
 * another packet waiting for its transmission or being transmitted
 * the new packet is stored in a queue. The length of this queue can
 * be specified by the user in the omnetpp.ini file. By default the
 * length is 10. The minimum queue length is 1. If the queue is full
 * new packet(s) will be deleted.
 *
 * @ingroup macLayer
 * @author Marc Löbbers, Yosia Hadisusanto, Andreas Koepke
 */
class  AlohaMacLayer : public BasicMacLayer
{
  public:
    Module_Class_Members(AlohaMacLayer, BasicMacLayer, 0);

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage*);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage *msg);

    /** @brief Called by the Blackboard whenever a change occurs we're interested in */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);

  protected:
    typedef std::list<MacPkt*> MacQueue;
    
    /** @brief MAC states
     *
     *  The MAC states help to keep track what the MAC is actually
     *  trying to do -- this is esp. useful when radio switching takes
     *  some time.
     *  RX  -- MAC accepts packets from PHY layer
     *  TX  -- MAC transmits a packet
     */
    enum States {
        RX,
        TX,
    };

    /** @brief kepp track of MAC state */
    States macState;
    
    /** @brief Current state of active channel (radio), set using radio, updated via BB */
    RadioState::States radioState;
    /** @brief category number given by bb for RadioState */
    int catRadioState;
    
    /** @brief cached pointer to radio module */
    SingleChannelRadio* radio;
    
    /** @brief A queue to store packets from upper layer in case another
    packet is still waiting for transmission..*/
    MacQueue macQueue;
    
    /** @brief length of the queue*/
    unsigned int queueLength;

    /** @brief Prepare sending of a packet (switch the radio) */
    void prepareSend();

};

#endif

