/* -*- mode:c++ -*- ********************************************************
 * file:        NicEntryDirect.h
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

#ifndef NICENTRYDIRECT_H
#define NICENTRYDIRECT_H

#include "NicEntry.h"


/**
 * @brief NicEntry is used by ChannelControl to store the necessary
 * information for each nic
 *
 * @ingroup channelControl
 * @author Daniel Willkomm
 * @sa ChannelControl, NicEntry
 */
class NicEntryDirect: public NicEntry
{
  public:
    /** @brief Constrcutor, initializes all members 
     * @todo initialize position!
     */
    NicEntryDirect(bool debug) : NicEntry(debug) {};
  
    /** 
     * @brief Destructor -- needs to be there...
     *
     */
    virtual ~NicEntryDirect() {}

    /** @brief Connect two nics */
    virtual void connectTo(NicEntry*);

    /** @brief Disconnect two nics */
    virtual void disconnectFrom(NicEntry*);
};

#endif
