/* -*- mode:c++ -*- ********************************************************
 * file:        P2PPhyLayer.h
 *
 * author:      Marc Loebbers, Daniel Willkomm
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
 * description: a very simple physical layer implementation
 ***************************************************************************/


#ifndef  P2P_PHYLAYER_H
#define  P2P_PHYLAYER_H

#include "ChannelAccess.h"
#include "AirFrame_m.h"
#include "NicControlType.h"
#include "ActiveChannel.h"

/**
 * @brief A very simple physical layer
 *
 * This is the simplest physical layer you can think of.
 *
 * It decides upon one simple parameter @ref pBit (bit error rate)
 * about the correctness of a frame. Collisions, bit errors,
 * interference and anything else you can think of are combined in
 * this single parameter.
 *
 * @ingroup phyLayer
 * @author Marc Loebbers, Daniel Willkomm
 */
class P2PPhyLayer : public ChannelAccess
{
  protected:
    /** @brief bitrate read from omnetpp.ini*/
    double bitrate;

    /**
     * @brief length of the AirFrame header 
     * 
     * @todo for now it is read from omnetpp.ini but should be easily
     * settable by the user later
     **/
    int headerLength;
  
    /** 
     * @brief bit error rate (including all other loss possibilities for
     * message losses)
     */
    double pBit;

    /** @brief power used to transmit messages */
    double transmitterPower;
  
    /** @brief gate ids*/
    /*@{*/
    int uppergateOut;
    int uppergateIn;
    int upperControlOut;
    /*@}*/

    /** @brief Timer to indicate the end of a transmission to a higher layer */
    cMessage *txOverTimer;

    /** @brief Currently active channel, set using radio, updated via BB */
    ActiveChannel channel;
    /** @brief category number given by bb for ActiveChannel */
    int catActiveChannel;

    /** @brief The kind field of messages
     * 
     * that are used internally by this class have one of these values
     * 
     */
    enum P2PPhyKinds {
        RECEPTION_COMPLETE = 1350101811
    };
    
    
  public:
    Module_Class_Members( P2PPhyLayer, ChannelAccess, 0 );
  
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();
  
    /** @brief Called every time a message arrives*/
    void handleMessage(cMessage*);

    /** @brief Called by the Blackboard whenever a change occurs we're interested in */
    virtual void receiveBBItem(int category, const BBItem *details, int scopeModuleId);
    
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
     * @brief Handle self messages such as timer... 
     *
     * Define this function if you want to timer or other kinds of self
     * messages
     **/
    virtual void handleSelfMsg(cMessage* msg){
        error("deleting msg");
        delete msg;
    };
    
    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(AirFrame*, int);
    
    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(AirFrame*);
    
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

    /** @brief Encapsulate the MacPkt into an AirFrame*/
    AirFrame* encapsMsg(cMessage*);
  
    /** @brief Buffer message upon reception*/
    void bufferMsg(AirFrame*);
  
    /** @brief Unbuffer message after it is completely received*/
    AirFrame* unbufferMsg(cMessage*);
  
    /** @brief Sends a unicast message; waits delay seconds before
        sending*/
    void sendP2P(AirFrame*, int);
  
    /** @brief Sends a message to the channel; waits delay seconds
        before sending*/
    void sendDown(AirFrame*, int);

    /** @brief Sends a message to the upper layer; waits delay seconds
        before sending*/
    void sendUp(AirFrame*);

    /** @brief Sends a unicast message*/
    void sendDelayedP2P(AirFrame*, double, int);
  
    /** @brief Sends a message to the channel */
    void sendDelayedDown(AirFrame*, double, int);

    /** @brief Sends a message to the upper layer*/
    void sendDelayedUp(AirFrame*, double);

    /** @brief Sends a control message to the upper layer*/
    void sendControlUp(cMessage *);

    /*@}*/

    /** @brief This function calculates the duration of the AirFrame */
    virtual double calcDuration(cMessage*);
    
    /**
     * @name Abstraction layer
     * @brief Factory function to create AirFrame into which a MAC frame is encapsulated.
     *
     * SnrEval authors should be able to use their own SnrEval packets. The
     * framework does not need to know about them, but must be able to
     * produce new ones. Both goals can be reached by using a factory
     * method.
     *
     * Overwrite this function in derived classes to use your
     * own AirFrames
     */
    /*@{*/

    /** @brief Create a new AirFrame */
    virtual AirFrame* createCapsulePkt() {
        return new AirFrame();
    };
    /*@}*/
    
};
#endif
