/* -*- mode:c++ -*- ********************************************************
 * file:        NicEntryDebug.cc
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


#include "NicEntryDebug.h"
#include "ChannelAccess.h"

#include <cassert>

#ifndef nicEV
#define nicEV (ev.disabled()||!coreDebug) ? (std::ostream&)ev : ev << "NicEntry: "
#endif


/**
 * Establish unidirectional connection with other nic
 *
 * @param other reference to remote nic (other NicEntry)
 *
 * This function acquires an in gate at the remote nic and an out
 * gate at this nic, connects the two and updates the freeInGate,
 * freeOutGate and outConns data sets.
 *
 * It handles compound modules correctly, provided that the physical
 * module is called "phy" or "snrEval" respectively in the .ned files.
 **/
void NicEntryDebug::connectTo(NicEntry* other)
{
    nicEV <<"connecting nic #"<<nicId<< " and #"<<other->nicId<<endl;
        
    NicEntryDebug* otherNic = (NicEntryDebug*) other;
        
    cGate *localoutgate = requestOutGate();
    localoutgate->connectTo(otherNic->requestInGate());
    outConns[other->nicId] = localoutgate->sourceGate();
}

/**
 * Release unidirectional connection with other nic
 *
 * @param other reference to remote nic (other NicEntry)
 **/
void NicEntryDebug::disconnectFrom(NicEntry* other)
{
    nicEV <<"disconnecting nic #"<<nicId<< " and #"<<other->nicId<<endl;
        
    NicEntryDebug* otherNic = (NicEntryDebug*) other;
        
    //search the connection in the outConns list
    std::map<int, cGate*>::iterator p = outConns.find(other->nicId);
    //no need to check whether entry is valid; is already check by ChannelControl isConnected
    //get the hostGate
    //order is phyGate->nicGate->hostGate
    cGate* hostGate = p->second->toGate()->toGate();
    
    // release local out gate 
    freeOutGates.push_back(hostGate); 
    
    // release remote in gate 
    otherNic->freeInGates.push_back(hostGate->toGate());
    
    //reset gates       
    hostGate->toGate()->setFrom(0);
    hostGate->setTo(0);
    
    //delete the connection
    outConns.erase(p);
}

/**
 *  This checks the list of free in gates, if one is available it is
 *  returned. Otherwise, a new in gate is added to the nic. This
 *  function handles the interaction with snrEval / phy and nic
 *  corectly, provided that the phy name is "snrEval or "phy"
 *  respectively in the .ned files.
 **/
cGate* NicEntryDebug::requestInGate(void)
{
    // gate of the host
    cGate *hostGate;

  if( !freeInGates.empty() ){
    hostGate = freeInGates.back();
    freeInGates.pop_back();
  }
  else {
    char gateName[20];
        
    // we will have one more in gate
    ++inCnt;

    //get a unique name for the gate (composed of the nic module id and a counter)
    sprintf(gateName,"in%d-%d",nicId,inCnt);


    // create a new gate for the host module
    hostGate = nicPtr->parentModule()->addGate(gateName, 'I');

    // gate of the nic
    cGate *nicGate;

    // create a new gate for the nic module
    nicGate = nicPtr->addGate(gateName, 'I');

    // connect the hist gate with the nic gate
    hostGate->connectTo(nicGate);

    // pointer to the phy module
    ChannelAccess* phyModule;
    // gate of the phy module
    cGate *phyGate;

    // if there is no snrEval module we should have a phy module (P2PPhyLayer)
    if( (phyModule = static_cast<ChannelAccess *>(nicPtr->submodule("snrEval"))) == NULL )
      phyModule = static_cast<ChannelAccess *>(nicPtr->submodule("phy"));
    assert(phyModule != 0);

    // create a new gate for the phy module
    phyGate = phyModule->addGate(gateName,'I');

    // connect the nic gate (the gate of the compound module) to
    // a "real" gate -- the gate of the phy module
    nicGate->connectTo(phyGate);
  }
  
  return hostGate;
}

/** 
 * returns a free out gate. If none is available it is created. See
 * NicEntry::requestInGate for a detailed description
 **/
cGate* NicEntryDebug::requestOutGate(void)
{
    // gate of the host
    cGate *hostGate;
    
    if( !freeOutGates.empty() ){
        hostGate = freeOutGates.back();
        freeOutGates.pop_back();
    }
    else {
        char gateName[20];
        
        // we will have one more out gate
        ++outCnt;
        
        //get a unique name for the gate (composed of the nic module id and a counter)
        sprintf(gateName,"out%d-%d",nicId,outCnt);
        
        
        // create a new gate for the host module
        hostGate = nicPtr->parentModule()->addGate(gateName, 'O');
        
        // gate of the nic
        cGate *nicGate;
        // create a new gate for the nic module
        nicGate = nicPtr->addGate(gateName, 'O');
        
        // connect the hist gate with the nic gate
        nicGate->connectTo(hostGate);
        
        // pointer to the phy module
        ChannelAccess* phyModule;
        // gate of the phy module
        cGate *phyGate;
        
        // if there is no snrEval module we should have a phy module (P2PPhyLayer)
        if( (phyModule = static_cast<ChannelAccess *>(nicPtr->submodule("snrEval"))) == NULL )
            phyModule = static_cast<ChannelAccess *>(nicPtr->submodule("phy"));
        assert(phyModule != 0);
        // create a new gate for the phy module
        phyGate = phyModule->addGate(gateName,'O');
        
        // connect the nic gate (the gate of the compound module) to
        // a "real" gate -- the gate of the phy module
        phyGate->connectTo(nicGate);
    }
    
    return hostGate;
}
