/* -*- mode:c++ -*- ********************************************************
 * file:        NicEntry.h
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
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
 * description: Class to store information about a nic for the 
 *              ChannelControl module
 **************************************************************************/

#ifndef NICENTRY_H
#define NICENTRY_H

#include <omnetpp.h>
#include <map>

#include "Coord.h"


/**
 * @brief NicEntry is used by ChannelControl to store the necessary
 * information for each nic
 *
 * @ingroup channelControl
 * @author Daniel Willkomm
 * @sa ChannelControl
 */
class NicEntry : public cPolymorphic
{
  public:
    typedef std::map<int, cGate*> GateList;
    
    /** @brief module id of the nic for which information is stored*/
    int nicId;
    
    /** @brief Pointer to the host module */
    cModule *nicPtr;
    
    /** @brief Module id of the host module this nic belongs to*/
    int hostId;
    
    /** @brief Geographic location of the nic*/
    Coord pos;

  protected:
    /** @brief Debug output switch*/
    bool coreDebug;
    
    /** @brief Outgoing connections of this nic 
     * 
     * This map stores all connection for this nic to other nics
     *
     * The first entry is the module id of the nic the connection is
     * going to and the second the gate to send the msg to
     **/
    GateList outConns;

  public:
    /** 
     * @brief Constrcutor, initializes all members
     *
     * @todo initialize position!
     */
    NicEntry(bool debug) : nicId(0), nicPtr(0), hostId(0){
        coreDebug = debug;
    };
    
    /** 
     * @brief Destructor -- needs to be there...
     *
     */
    virtual ~NicEntry() {}
    
    /** @brief Connect two nics */
    virtual void connectTo(NicEntry*) = 0;
    
    /** @brief Disconnect two nics */
    virtual void disconnectFrom(NicEntry*) = 0;

    /** @brief return the actual gateList*/
    const GateList& getGateList(){
	return outConns;
    }

    /** @brief Checks if this nic is connected to the "other" nic*/
    bool isConnected(const NicEntry* other) {
        return (outConns.find(other->nicId) != outConns.end());
    };
  
    /**
     * Called by P2PPhyLayer. Needed to send a packet directly to a
     * certain nic without other nodes 'hearing' it. This is only useful
     * for physical layers that work with bit error probability like
     * P2PPhyLayer.
     *
     * @param to id of the nic to which the packet is about to be sent
     */
    const cGate* getOutGateTo(int to)
    {
	return outConns[to];
    };

};

#endif
