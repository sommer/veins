/* -*- mode:c++ -*- ********************************************************
 * file:        Flood.h
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
 *
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: a simple flooding protocol
 *              the user can decide whether to use plain flooding or not
 **************************************************************************/


#ifndef _FLOOD_H_
#define _FLOOD_H_

#include <list>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "SimpleAddress.h"

/**
 * @brief A simple flooding protocol
 *
 * This implementation uses plain flooding, i.e. it "remembers"
 * (stores) already broadcasted messages in a list and does not
 * rebroadcast them again, if it gets another copy of that message.
 *
 * The maximum number of entires for that list can be defined in the
 * .ini file (@ref bcMaxEntries) as well as the time after which an entry
 * is deleted (@ref bcDelTime).
 *
 * If you prefere a memory-less version you can comment out the 
 * @verbatim #define PLAINFLOODING @endverbatim
 *
 * @ingroup netwLayer
 * @author Daniel Willkomm
 *
 * ported to Mixim 2.0 by Theodoros Kapourniotis
 **/
class MIXIM_API Flood : public BaseNetwLayer
{
protected:
    /** @brief Network layer sequence number*/
    unsigned long seqNum;

    /** @brief Default time-to-live (ttl) used for this module*/
    int defaultTtl;

    /** @brief Defines whether to use plain flooding or not*/
    bool plainFlooding;

    class Bcast {
    public:
        unsigned long    seqNum;
        LAddress::L3Type srcAddr;
        simtime_t        delTime;
    public:
        Bcast(unsigned long n=0, const LAddress::L3Type& s = LAddress::L3NULL,  simtime_t_cref d=SIMTIME_ZERO) :
            seqNum(n), srcAddr(s), delTime(d) {
        }
    };

    typedef std::list<Bcast> cBroadcastList;

    /** @brief List of already broadcasted messages*/
    cBroadcastList bcMsgs;

    /**
     * @brief Max number of entries in the list of already broadcasted
     * messages
     **/
    unsigned int bcMaxEntries;

    /** 
     * @brief Time after which an entry for an already broadcasted msg
     * can be deleted
     **/
    simtime_t bcDelTime;

public:


    /** @brief Initialization of omnetpp.ini parameters*/
    virtual void initialize(int);
    virtual void finish();    

protected:

    long nbDataPacketsReceived;
    long nbDataPacketsSent;
    long nbDataPacketsForwarded;
    long nbHops;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage *);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage *);

    /** @brief we have no self messages */
    virtual void handleSelfMsg(cMessage* msg) {};
    
    /** @brief Checks whether a message was already broadcasted*/
    bool notBroadcasted( NetwPkt* );

    //overloading encaps method
    NetwPkt* encapsMsg(cPacket*);
};

#endif
