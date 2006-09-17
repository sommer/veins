/* -*- mode:c++ -*- ********************************************************
 * file:        Flood.cc
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
 * description: a simple flooding protocol
 *              the user can decide whether to use plain flooding or not
 **************************************************************************/


#include "Flood.h"
#include <NetwPkt_m.h>
#include "MacControlInfo.h"

Define_Module(Flood);


/**
 * Reads all parameters from the ini file. If a parameter is not
 * specified in the ini file a default value will be set.
 **/
void Flood::initialize(int stage)
{
    SimpleNetwLayer::initialize(stage);
  
    if(stage==0){
        //initialize seqence number to 0
        seqNum = 0;
    
        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 5;
        hasPar("plainFlooding") ? plainFlooding = par("plainFlooding") : plainFlooding = true;

        EV << "defaultTtl = " << defaultTtl
           << " plainFlooding = " << plainFlooding << endl;
        
        if(plainFlooding){
            //these parameters are only needed for plain flooding
            hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;
      
            hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 3.0;
            EV <<"bcMaxEntries = "<<bcMaxEntries
               <<" bcDelTime = "<<bcDelTime<<endl;
        }
    }
}

void Flood::finish() {
    if(plainFlooding){
	bcMsgs.clear();
    }
}

/**
 * All messages have to get a sequence number and the ttl filed has to
 * be specified. Afterwards the messages can be handed to the mac
 * layer. The mac address is set to -1 (broadcast address) because the
 * message is flooded (i.e. has to be send to all neighbors)
 *
 * In the case of plain flooding the message sequence number and
 * source address has also be stored in the bcMsgs list, so that this
 * message will not be rebroadcasted, if a copy will be flooded back
 * from the neigbouring nodes.
 *
 * If the maximum number of entries is reached the first (oldest) entry
 * is deleted.
 **/
void Flood::handleUpperMsg(cMessage* m)
{

    NetwPkt *msg = encapsMsg(m);

    msg->setSeqNum( seqNum );
    seqNum++;
    msg->setTtl( defaultTtl );

    if(plainFlooding){
        if( bcMsgs.size() >= bcMaxEntries ){
            cBroadcastList::iterator it;
      
            //serach the broadcast list of outdated entries and delete them
            for( it=bcMsgs.begin(); it!=bcMsgs.end(); ++it ){
                if( it->delTime < simTime() ){
                    bcMsgs.erase(it);
                    it--;
                    break;
                }
            }
            //delete oldest entry if max size is reached
            if( bcMsgs.size() >= bcMaxEntries ){
                EV <<"bcMsgs is full, delete oldest entry"<<endl;
                bcMsgs.pop_front();
            }
        }
        bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getSrcAddr(), simTime() + bcDelTime));
    }
    //there is no routing so all messages are broadacst for the mac layer
    sendDown(msg);
}

/**
 * Messages from the mac layer will be forwarded to the application
 * only if the are broadcast or destined for this node.
 *
 * If the arrived message is a broadcast message it is also reflooded
 * only if the tll field is bigger than one. Before the message is
 * handed back to the mac layer the ttl field is reduced by one to
 * account for this hop.
 *
 * In the case of plain flooding the message will only be processed if
 * there is no corresponding entry in the bcMsgs list (@ref
 * notBroadcasted). Otherwise the message will be deleted.
 **/
void Flood::handleLowerMsg(cMessage* m)
{
    NetwPkt *msg = static_cast<NetwPkt *>(m);
    
    //msg not broadcastes yet
    if( notBroadcasted( msg ) ){
        //msg is for me
        if( msg->getDestAddr() == myNetwAddr ){
            EV <<" data msg for me! send to Upper"<<endl;
            sendUp( decapsMsg(msg) );
        }
        //broadcast message
        else if( msg->getDestAddr() == L3BROADCAST ){
            //check ttl and rebroadcast
            if( msg->getTtl() > 1 ){
		NetwPkt *dMsg;
                EV <<" data msg BROADCAST! ttl = "<<msg->getTtl()
                   <<" > 1 -> rebroadcast msg & send to upper\n";
                msg->setTtl( msg->getTtl()-1 );
                dMsg = static_cast<NetwPkt*>(msg->dup());
                dMsg->setControlInfo(new MacControlInfo(L2BROADCAST));
                sendDown(dMsg);
            }
            else
                EV <<" max hops reached (ttl = "<<msg->getTtl()<<") -> only send to upper\n";

	    // message has to be forwarded to upper layer
	    sendUp( decapsMsg(msg) );
        }
        //not for me -> rebroadcast
        else{
            //check ttl and rebroadcast
            if( msg->getTtl() > 1 ){
                EV <<" data msg not for me! ttl = "<<msg->getTtl()
                   <<" > 1 -> forward\n";
                msg->setTtl( msg->getTtl()-1 );
                sendDown( msg );
            }
            else{
                //max hops reached -> delete
                EV <<" max hops reached (ttl = "<<msg->getTtl()<<") -> delete msg\n";
                delete msg;
            }
        }
    }
    else{
        EV <<" data msg already BROADCASTed! delete msg\n";
        delete msg;
    }
}

/**
 * The bcMsgs list is searched for the arrived message. If the message
 * is in the list, it was already broadcasted and the function returns
 * false.
 *
 * Concurrently all outdated (older than bcDelTime) are deleted. If
 * the list is full and a new message has to be entered, the oldest
 * entry is deleted.
 **/
bool Flood::notBroadcasted( NetwPkt* msg )
{
    if(!plainFlooding)
        return true;
  
    cBroadcastList::iterator it;

    //serach the broadcast list of outdated entries and delete them
    for( it=bcMsgs.begin(); it!=bcMsgs.end(); it++ ){
        if( it->delTime < simTime() ){
            bcMsgs.erase(it);
            it--;
        }
        //message was already broadcasted
        if( (it->srcAddr==msg->getSrcAddr()) && (it->seqNum==msg->getSeqNum()) ){
            // update entry
            it->delTime = simTime() + bcDelTime;
            return false;
        }
    }
  
    //delete oldest entry if max size is reached
    if( bcMsgs.size() >= bcMaxEntries ){
        EV <<"bcMsgs is full, delete oldest entry\n";
        bcMsgs.pop_front();
    }
    
    bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getSrcAddr(), simTime() + bcDelTime));
    return true;
}
