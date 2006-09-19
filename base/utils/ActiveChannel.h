/* -*- mode:c++ -*- ********************************************************
 * file:        ActievChannel.h
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

#ifndef ACTIVECHANNEL_H
#define ACTIVECHANNEL_H

#include <omnetpp.h>
#include <Blackboard.h>
#include <sstream>

/**
 * @brief Announce the current channel that we send on.
 *
 *
 * @ingroup basicUtils
 * @ingroup utils
 * @ingroup blackboard
 * @author Andreas Köpke
 * @sa Blackboard
 */

class  ActiveChannel : public BBItem
{
    BBITEM_METAINFO(BBItem);

protected:
    /** @brief id of currently active channel */
    unsigned channel;
    
public:
    
    /** @brief Constructor*/
    ActiveChannel(unsigned c=0) : BBItem(), channel(c) {
    };

    /** @brief get active channel */
    int getActiveChannel() const  {
        return channel;
    }
    
    /** @brief set active channel */
    void setActiveChannel(unsigned c) {
        channel = c;
    }
    
    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << "Active channel is " << channel;
        return ost.str();
    }
};

#endif
