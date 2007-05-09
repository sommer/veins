/***************************************************************************
 * file:        ChannelAccess.cc
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
 * description: - Base class for physical layers
 *              - if you create your own physical layer, please subclass 
 *                from this class and use the sendToChannel() function!!
 ***************************************************************************
 * changelog:   $Revision: 284 $
 *              last modified:   $Date: 2006-06-07 16:55:24 +0200 (Mi, 07 Jun 2006) $
 *              by:              $Author: willkomm $
 **************************************************************************/


#include "ChannelAccess.h"

#include "ChannelControl.h"
#include "Move.h"

/**
 * Upon initialization ChannelAccess registers the nic parent module
 * to have all its connections handeled by ChannelControl
 **/
void ChannelAccess::initialize( int stage )
{
    BaseModule::initialize(stage);

    if( stage == 0 ){
        hasPar("coreDebug") ? coreDebug = par("coreDebug").boolValue() : coreDebug = false;

        cc = dynamic_cast<ChannelControl *>(getGlobalModule("ChannelControl"));
        if( cc == 0 ) error("Could not find channelcontrol module");

        // subscribe to position changes
        //catMove = bb->subscribe(this, &move, findHost()->id());
		Coord where;
        if (hasPar("x") && hasPar("y") && hasPar("z")){
            where.x = par("x");
            where.y = par("y");
			where.z = par("z");
        }
        else{
            // Start at a random position
			where.x = where.y = where.z = -1;
        }
		cc->registerNic(parentModule(), &where);
        isRegistered = false;
    }
}


/**
 * This function has to be called whenever a packet is supposed to be
 * sent to the channel. Don't try to figure out what gates you have
 * and which ones are connected, this function does this for you!
 *
 * depending on which ChannelControl module is used, the messages are
 * send via sendDirect() or to the respective gates.
 **/
void ChannelAccess::sendToChannel(cMessage *msg, double delay)
{
    const NicEntry::GateList& gateList = cc->getGateList( parentModule()->id(), &move.startPos );
    NicEntry::GateList::const_iterator i = gateList.begin();
        
    if(useSendDirect){
        // use Andras stuff
        if( i != gateList.end() ){
            for(; i != --gateList.end(); ++i){

                int radioStart = i->second->id();
                int radioEnd = radioStart + i->second->size();
                for (int g = radioStart; g != radioEnd; ++g)
                    sendDirect(static_cast<cMessage*>(msg->dup()),
                               delay, i->second->ownerModule(), g);
            }
            int radioStart = i->second->id();
            int radioEnd = radioStart + i->second->size();
            for (int g = radioStart; g != --radioEnd; ++g)
                sendDirect(static_cast<cMessage*>(msg->dup()),
                           delay, i->second->ownerModule(), g);
            
            sendDirect(msg, delay, i->second->ownerModule(), radioEnd);
        }
        else{
            coreEV << "Nic is not connected to any gates!" << endl;
            delete msg;
        }
    }
    else{
        // use our stuff
        coreEV <<"sendToChannel: sending to gates\n";
        if( i != gateList.end() ){
            for(; i != --gateList.end(); ++i){
                sendDelayed( static_cast<cMessage*>(msg->dup()),
                             delay, i->second );
            }
            sendDelayed( msg, delay, i->second );
        }
        else{
            coreEV << "Nic is not connected to any gates!" << endl;
            delete msg;
        }
    }
}

/**
 * ChannelAccess is subscribed to position changes and informs the
 * channelcontrol
 */
/*void ChannelAccess::receiveBBItem(int category, const BBItem *details, int scopeModuleId) 
{    
    //BaseModule::receiveBBItem(category, details, scopeModuleId);
    
    if(category == catMove)
    {
        Move m(*static_cast<const Move*>(details));
        
        if(isRegistered) {
            cc->updateNicPos(parentModule()->id(), &move.startPos, &m.startPos);
        }
        else {
            // register the nic with ChannelControl
            // returns true, if sendDirect is used
            useSendDirect = cc->registerNic(parentModule(), &m.startPos);
            isRegistered = true;
        }
        move = m;
        coreEV<<"new HostMove: "<<move.info()<<endl;
    }
}*/

