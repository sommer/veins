/* -*- mode:c++ -*- ********************************************************
 * file:        BasePhyLayer.h
 *
 * author:      Marc Loebbers
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


#ifndef BASE_PHY_LAYER_H
#define BASE_PHY_LAYER_H

#include "BaseLayer.h"
#include "AirFrame_m.h"
#include "Timer.h"
#include "BasePropagation.h"

/**
 * @brief The basic class for all phy modules
 *
 * The BasePhyLayer module provides functionality like en- and
 * decapsulation of messages. If you use the standard message formats
 * everythng should work fine. Before a packet is sent some
 * information, e.g. transmission power, can be written to the
 * AirFrame header. If you write your own snrEval, just subclass and
 * redefine the handleUpperMsg function (see description of the
 * function). After receiving a message it can be processed in
 * handleLowerMsgStart. Then it is buffered for the time the
 * transmission would last in reality, and then can be handled
 * again. Again you can redefine the 1. handleLowerMsgStart and
 * 2. handleLowerMsgEnd for your own needs (see description). So, the
 * call of these functions represent the following events: 1. received
 * a message (i.e. transmission startet) 2. message will be handed on
 * to the upper layer (i.e. transmission time is over)
 *
 * @author Marc Loebbers
 * @author Andreas Koepke
 * @author Daniel Willkomm
 * @ingroup snrEval
 * @ingroup basicModules
 */
class BasePhyLayer : public BaseLayer, public Timer
{

protected:

    /** @brief a parameter that has to be read in from omnetpp.ini*/
    int headerLength;

    /** @brief debug this core module? */
    bool coreDebug;

	BasePropagation *pm;
    
public:
    Module_Class_Members( BasePhyLayer, BaseLayer, 0 );

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    void handleLowerMsg( cMessage* );
	void handleTimer(unsigned int index);

protected:
    /**
     * @name Handle Messages
     * @brief Functions to redefine by the programmer
     *
     * These are the functions provided to add own functionality to your
     * modules. These functions are called whenever a blackboard
     * message, a self message or a data message from the upper or lower
     * layer arrives respectively.
     *
     **/
    /*@{*/

    /**
     * @brief Fill the header fields, redefine for your own needs...
     */
    virtual void handleUpperMsg(cMessage*);
    void handleLowerControl( cMessage* msg)
	{
		error("BasePhyLayer doesn't handle lower control messages\n");
	}

    /**
     * @brief Handle self messages such as timer...
     *
     * Define this function if you want to process timer or other kinds
     * of self messages
     */
    virtual void handleSelfMsg(cMessage *msg){
        error("no self messages in BasePhyLayer");
    };

    /**
     * @brief Fill the header fields, redefine for your own needs...
     */
    virtual void handleUpperControl(cMessage *msg){
        error("no control messages in BasePhyLayer");
    };


    /**
     * @name Convenience Functions
     * @brief Functions for convenience
     *
     * These are functions taking care of message encapsulation and
     * message sending. Normally you should not need to alter these but
     * should use them to handle message encasulation and sending. They
     * will wirte all necessary information into packet headers and add
     * or strip the appropriate headers for each layer.
     *
     */
    /*@{*/

    /** @brief Sends a message to the channel*/
    virtual void sendDown(AirFrame *msg);

    /** @brief Encapsulates a MAC packet into an AirFrame*/
    virtual AirFrame* encapsMsg(cMessage *msg);

    /** @brief Decapsulates the MAC packet from an AirFrame*/
    cMessage* decapsMsg(AirFrame* frame);

    /*@}*/
};

#endif
