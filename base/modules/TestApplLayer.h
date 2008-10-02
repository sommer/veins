/* -*- mode:c++ -*- ********************************************************
 * file:        TestApplLayer.h
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
 * description: application layer: test class for the application layer
 **************************************************************************/


#ifndef TEST_APPL_LAYER_H
#define TEST_APPL_LAYER_H

#include "BaseApplLayer.h"


/**
 * @brief Test class for the application layer
 * 
 * In this implementation all nodes randomly send broadcast packets to
 * all connected neighbors. Every node who receives this packet will
 * send a reply to the originator node.
 *
 * @ingroup applLayer
 * @author Daniel Willkomm
 **/
class TestApplLayer : public BaseApplLayer
{
  public:
    //Module_Class_Members( TestApplLayer, BaseApplLayer, 0 );

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    enum APPL_MSG_TYPES{
	SEND_BROADCAST_TIMER,
	BROADCAST_MESSAGE,
	BROADCAST_REPLY_MESSAGE
    };

protected:
    cMessage *delayTimer;
    
protected:
    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage*);
  
    /** @brief send a broadcast packet to all connected neighbors */
    void sendBroadcast();

    /** @brief send a reply to a broadcast message */
    void sendReply(ApplPkt *msg);
};

#endif
