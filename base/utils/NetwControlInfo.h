/* -*- mode:c++ -*- *******************************************************
 * file:        NetwControlInfo.h
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
 **************************************************************************
 * part of:     framework implementation developed by tkn
 * description: - control info to pass the netw addresses between the 
 *                network and application layer
 **************************************************************************/

#ifndef APPLCONTROLINFO_H
#define APPLCONTROLINFO_H

#include <omnetpp.h>

/**
 * @brief Control info netw messages
 * 
 * Control Info to pass interface information from the network to the
 * application layer and vice versa. The application layer passes the
 * destination netw address to the network layer, whereas the network
 * layer uses the control info to pass the source netw address to the
 * application layer
 *
 * @ingroup utils
 * @author Daniel Willkomm
 **/
class NetwControlInfo : public cPolymorphic
{
  protected:
    /** @brief netw address of the sending or receiving node*/
    int netwAddr;

  public:
    /** @brief Default constructor*/
    NetwControlInfo(int addr = 0) : netwAddr(addr) {};
    /** @brief Destructor*/
    virtual ~NetwControlInfo(){};

    /** @brief Getter method*/
    virtual const int getNetwAddr(){
	return netwAddr;
    };

    /** @brief Setter method*/
    virtual void setNetwAddr(const int addr){
	netwAddr = addr;
    };

};


#endif
