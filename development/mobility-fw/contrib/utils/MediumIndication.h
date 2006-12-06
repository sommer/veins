/* -*- mode:c++ -*- ********************************************************
 * file:        MediumIndication.h
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

#ifndef MEDIUM_INDICATION_H
#define MEDIUM_INDICATION_H

#include <omnetpp.h>
#include <Blackboard.h>
#include <sstream>

/**
 * @brief What is the current medium state? busy vs. idle, not valid when radio state is sending or sleeping
 *
 * @ingroup utils
 * @ingroup blackboard
 * @author Andreas Köpke
 * @sa Blackboard
 */

class MediumIndication : public BBItem
{
    BBITEM_METAINFO(BBItem);
    
 public:
    /** @brief possible states of medium*/
    enum States {
        IDLE,
        BUSY
    };
    
 protected:
    States state;
    
 public:
    /** @brief Constructor*/
    MediumIndication(States s=IDLE) : BBItem(), state(s) {
    };

    /** @brief get current medium state */
    States getState() const  {
        return state;
    }
    
    /** @brief set current medium state */
    void setState(States s) {
        state = s;
    }
    
    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << " medium is ";
        if(state == IDLE) {
            ost << "idle";
        }
        else {
            ost << "busy";   
        }
        return ost.str();
    }
};


#endif
