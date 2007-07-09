/* -*- mode:c++ -*- ********************************************************
 * file:        CollisionsPhy.h
 * 
 * author:      Tom Parker
 *
 * copyright:   (C) 2006 Parallel and Distributed Systems Group (PDS) at
 *              Technische Universiteit Delft, The Netherlands.
 *
 *              This program is free software; you can redistribute it 
 *              and/or modify it under the terms of the GNU General Public 
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later 
 *              version.
 *              For further information see file COPYING 
 *              in the top level directory
 * description: propagation layer - collisions/start msg capable
 ***************************************************************************/

#ifndef COLLISIONS_PHY_LAYER_H
#define COLLISIONS_PHY_LAYER_H

#include "BasePhyLayer.h"
#include "StartMessage_m.h"
#include "CorruptMessage_m.h"

class CollisionsPhy : public BasePhyLayer
{

public:
    Module_Class_Members(CollisionsPhy, BasePhyLayer, 0 );

	void initialize(int stage);
    void handleLowerMsg( cMessage* );

protected:

    /** @brief keep bitrate to calculate duration */
    double bitrate;

	void handleUpperMsg(cMessage *msg);

    /** 
     *
     * This function is called right after a message is received,
     * i.e. right before it is buffered for 'transmission time'.
     *
     * Here you should decide whether the message is "really" received
     * or whether it's receive power is so low that it is just treated
     * as noise.
     **/
    virtual void handleLowerMsgStart(StartMessage*);

    /**
     * @brief Calculate SnrInfo after buffering and add the PhySnrList
     * to the message
     *
     * Redefine this function if you want to process messages from the
     * channel before they are forwarded to upper layers
     */
    virtual void handleLowerMsgEnd(AirFrame*);

	virtual void handleCorruptMessage(CorruptMessage*);
	
	/** @brief Calculate duration of this message */
    virtual double calcDuration(cMessage* m) {
        return static_cast<double>(m->length()) / bitrate;
    }

};

#endif
