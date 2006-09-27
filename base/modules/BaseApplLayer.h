/* -*- mode:c++ -*- ********************************************************
 * file:        BaseApplLayer.h
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
 * description: application layer: general class for the application layer
 *              subclass to create your own application layer
 **************************************************************************/


#ifndef BASE_APPL_LAYER_H
#define BASE_APPL_LAYER_H

#include "BaseModule.h"
#include "ApplPkt_m.h"


/**
 * @brief Base class for the application layer
 * 
 * This is the generic class for all application layer modules. If you
 * want to implement your own application layer you have to subclass your
 * module from this class.
 *
 * @ingroup applLayer
 *
 * @author Daniel Willkomm
 **/
class BaseApplLayer : public BaseModule
{
  protected:
    /** @brief gate id*/
    /*@{*/
    int lowergateIn;
    int lowergateOut;
    int lowerControlIn;
    int lowerControlOut;
    /*@}*/  

    /**
     * @brief Length of the ApplPkt header 
     * 
     * @todo for now it is read from omnetpp.ini but should be easily
     * settable by the user later
     **/
    int headerLength;

  public:
    Module_Class_Members( BaseApplLayer , BaseModule, 0 );

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

    /** @brief Called every time a message arrives*/
    void handleMessage(cMessage*);

  protected:
    /** 
     * @name Handle Messages
     * @brief Functions to redefine by the programmer
     *
     * These are the functions provided to add own functionality to your
     * modules. These functions are called whenever a self message or a
     * data message from the upper or lower layer arrives respectively.
     *
     **/
    /*@{*/

    /** 
     * @brief Handle self messages such as timer...
     *
     * Define this function if you want to process timer or other kinds
     * of self messages
     **/
    virtual void handleSelfMsg(cMessage* msg){
	EV << "BaseAppl: handleSelfMsg not redefined; delete msg\n";
	delete msg;
    };

    /** 
     * @brief Handle messages from lower layer
     *
     * Redefine this function if you want to process messages from lower
     * layers.
     *
     * The basic application layer just silently deletes all messages it
     * receives.
     **/
    virtual void handleLowerMsg(cMessage* msg){
	EV << "BaseAppl: handleLowerMsg not redefined; delete msg\n";
	delete msg;
    };

    /** 
     * @brief Handle control messages from lower layer
     *
     * The basic application layer just silently deletes all messages it
     * receives.
     **/
    virtual void handleLowerControl(cMessage* msg){
	EV << "BaseAppl: handleLowerControl not redefined; delete msg\n";
	delete msg;
    };

    /*@}*/

    /** 
     * @name Convenience Functions
     * @brief Functions for convenience - NOT to be modified
     *
     * These are functions taking care of message encapsulation and
     * message sending. Normally you should not need to alter these but
     * should use them to handle message encasulation and sending. They
     * will wirte all necessary information into packet headers and add
     * or strip the appropriate headers for each layer.
     *
     **/
    /*@{*/

    /** @brief Sends a message to the lower layer*/
    void sendDown(cMessage *);
    
    /** @brief Sends a message delayed to the lower layer*/
    void sendDelayedDown(cMessage *, double);
    
    /** @brief Sends a control message to the lower layer*/
    void sendControlDown(cMessage *);
    
    /*@}*/
    

    /** 
     * @brief Return my application layer address
     * 
     * We use the node module index as application address
     **/
    virtual const int myApplAddr(){
	return parentModule()->index();
    };
    
};

#endif
