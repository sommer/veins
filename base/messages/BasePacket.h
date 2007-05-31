/* -*- mode:c++ -*- ********************************************************
 * file:        BasePacket.cc
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
 ***************************************************************************
 * part of:     message classes
 * description: packet utility class
 ***************************************************************************/

#ifndef BASE_PACKET_H
#define BASE_PACKET_H 1

#include <omnetpp.h>

class BasePacket: public cMessage
{
   public:
    BasePacket(const char *name=NULL, int kind=0) : cMessage(name,kind) {}
    BasePacket(const BasePacket& other) : cMessage(other.name()) {operator=(other);}

	simtime_t simTime () const;
    std::string logName(void) const;
  protected:	
    bool debug;
    cModule *findHost(void) const;
};

#endif

