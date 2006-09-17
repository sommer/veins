/* -*- mode:c++ -*- ********************************************************
 * file:        NicControlType.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2006 Telecommunication Networks Group (TKN) at
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
 **************************************************************************/

#ifndef NICCONTROLTYPE_H
#define NICCONTROLTYPE_H

#include <omnetpp.h>

/**
 * @brief Enumerate possible control messages
 *
 * Enumerate for the kind field for control messages exchanged within
 * a NIC
 *
 * @ingroup basicUtils
 * @ingroup utils
 * @author Andreas Koepke
 */

class NicControlType : public cPolymorphic
{    
 public:
    enum Types {
        NOTHING, // we don't want to start at zero -- its is ambiguous
        TRANSMISSION_OVER,
        PACKET_DROPPED
    };
    

    /** @brief Enables inspection
     *
     * If you want to know the description of a certain number, make
     * this object out of the number and call info. 
     * 
     * @{ */ 
    NicControlType(Types t=NOTHING) : cPolymorphic(), type(t) {
        ;
    };
    
    int getType() const  {
        return type;
    }
    
    void setType(Types t) {
        type = t;
    }
    
    std::string info() const {
        std::ostringstream ost;
        if(type == NOTHING) {
            ost<<"NOTHING";
        }
        else if(type == TRANSMISSION_OVER) {
            ost<<"TRANSMISSION_OVER";
        }
        else {
            ost<<"UNKNOWN TYPE";
        }
        return ost.str();
    }
    /* @} */

protected:
    Types type;

};

#endif
