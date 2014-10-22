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
 *              ConnectionManager module
 **************************************************************************/

#ifndef NICENTRYDIRECT_H
#define NICENTRYDIRECT_H

#include "veins/base/connectionManager/NicEntry.h"


/**
 * @brief NicEntry is used by ConnectionManager to store the necessary
 * information for each nic
 *
 * @ingroup connectionManager
 * @author Daniel Willkomm
 * @sa ConnectionManager, NicEntry
 */
class NicEntryDirect: public NicEntry
{
  public:
    /** @brief Constructor, initializes all members
     */
    NicEntryDirect(bool debug) : NicEntry(debug) {};

    /**
     * @brief Destructor -- needs to be there...
     */
    virtual ~NicEntryDirect() {}

    /** @brief Connect two nics
     *
	 * Establish unidirectional connection with other nic
	 *
	 * @param other reference to remote nic (other NicEntry)
	 *
	 * This function acquires an in gate at the remote nic and an out
	 * gate at this nic, connects the two and updates the freeInGate,
	 * freeOutGate and outConns data sets.
	 */
    virtual void connectTo(NicEntry*);

    /** @brief Disconnect two nics
     *
     * Release unidirectional connection with other nic
     *
     * @param other reference to remote nic (other NicEntry)
     */
    virtual void disconnectFrom(NicEntry*);
};

#endif
