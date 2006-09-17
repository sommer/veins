/* -*- mode:c++ -*- *******************************************************
 * file:        BasicDecider.h
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
 **************************************************************************/


#ifndef  BASIC_DECIDER_H
#define  BASIC_DECIDER_H

#include <omnetpp.h>

#include "BasicModule.h"
#include "AirFrame_m.h"


/**
 * @brief Module to decide whether a frame is received correctly or is
 * lost due to bit errors, interference...
 *
 * The decider module only handles messages from lower layers. All
 * messages from upper layers are directly passed to the snrEval layer
 * and cannot be processed in the decider module
 *
 * This is the basic decider module which does not really decide
 * anything. It only provides the basic functionality which all
 * decider modules should have, namely message de- & encapsulation
 * (For further information about the functionality of the physical
 * layer modules and the formats used for communication in between
 * them have a look at "The Design of a Mobility Framework in OMNeT++"
 * paper)
 *
 * Every own decider module class should be derived from this class
 * and only the handle*Msg functions may be redefined for your own
 * needs. The other functions should usually NOT be changed.
 *
 * All decider modules should assume bits as a unit for the length
 * fields.
 *
 * @ingroup decider
 * @ingroup basicModules
 * @author Marc Löbbers, Daniel Willkomm
 */
class BasicDecider : public BasicModule
{
    Module_Class_Members(BasicDecider,BasicModule,0);

protected:
    /** @brief gate id*/
    /*@{*/
    int uppergateOut;
    int lowergateIn;
    /*@}*/

    /** @brief debug this core module? */
    bool coreDebug;

public:
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Called every time a message arrives*/
    void handleMessage( cMessage* );

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
     */
    /*@{*/

    /**
     * @brief Handle self messages such as timer...
     *
     * Define this function if you want to timer or other kinds of self
     * messages
     */
    virtual void handleSelfMsg(cMessage *msg) = 0;

    /**
     * @brief In this function the decision whether a frame is received
     * correctly or not is made.
     *
     * Redefine this function if you want to process messages from the
     * channel before they are forwarded to upper layers
     *
     * In this function it has to be decided whether this message got lost
     * or not. This can be done with a simple SNR threshold or with
     * transformations of SNR into bit error probabilities...
     *
     * Afterwrads the message has to be decapsulated (@ref decapsMsg)
     * and send to the MAC layer (@ref sendUp)
     *
     * just an example how to handle the snrList...:
     *
     *   bool correct = true;
     *   //check the entries in the snrList if a level greater than the
     *   //acceptable minimum occured:
     *   double min = 0.0000000000000001;    // just a senseless example
     *
     *   for (SnrList::iterator iter = snrList.begin(); iter != snrList.end(); iter++){
     *     if (iter->snr < min)
     *       correct = false;
     *   }
     *
     *   if (correct){
     *     sendUp(frame);
     *   }
     *   else{
     *     delete frame;
     *   }
     *
     */
    virtual void handleLowerMsg(AirFrame*, const SnrList&) = 0;
    
    /*@}*/

    /** @brief Sends a message to the upper layer*/
    void sendUp(cMessage *);

    /** @brief decapsulate the MacPkt from the AirFrame*/
    virtual cMessage* decapsMsg(AirFrame*);


};
#endif


