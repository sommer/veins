/* -*- mode:c++ -*- ********************************************************
 * file:        NicEntryDebug.h
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

#ifndef NICENTRYDEBUG_H
#define NICENTRYDEBUG_H

#include "NicEntry.h"

#include <map>
#include <vector>

/**
 * @brief NicEntry is used by ChannelControl to store the necessary
 * information for each nic
 *
 * @ingroup channelControl
 * @author Daniel Willkomm
 * @sa ChannelControl, NicEntry
 */
class NicEntryDebug: public NicEntry
{
  protected:
    /** @brief Number of in gates allocated for the nic so far*/
    int inCnt;
    
    /** @brief Number of out gates allocated for the nic so far */
    int outCnt;
    
    /** @brief In Gates that were once used but are not connected now */
    std::vector<cGate* > freeInGates;
    
    /** @brief Out Gates that were once used but are not connected now */
    std::vector<cGate* > freeOutGates;
        
  protected:
    /** @brief Returns a free in gate of the nic */
    cGate* requestInGate(void);
        
    /** @brief Returns a free out gate of the nic */
    cGate* requestOutGate(void);
        
  public:
    /** @brief Constrcutor, initializes all members 
     * @todo initialize position!
     */
    NicEntryDebug(bool debug) : NicEntry(debug), inCnt(0), outCnt(0) {};
  
    /** 
     * @brief Destructor -- needs to be there...
     *
     */
    virtual ~NicEntryDebug() {}

    /** @brief Connect two nics */
    virtual void connectTo(NicEntry*);
    
    /** @brief Disconnect two nics */
    virtual void disconnectFrom(NicEntry*);
};

#endif
