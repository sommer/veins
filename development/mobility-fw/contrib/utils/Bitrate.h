/* -*- mode:c++ -*- ********************************************************
 * file:        Bitrate.h
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

#ifndef BITRATE_H
#define BITRATE_H

#include <omnetpp.h>
#include <Blackboard.h>
#include <sstream>

/**
 * @brief Hold current Bitrate of a radio, as double in bit per second
 *
 * @ingroup utils
 * @ingroup blackboard
 * @author Andreas Köpke
 * @sa Blackboard
 */
class  Bitrate : public BBItem
{
    BBITEM_METAINFO(BBItem);
    
protected:
    /** @brief current bitrate in bit/s */
    double bitrate;
    
public:

    /** @brief Get current bit rate */
    double getBitrate () const {
        return bitrate;
    }
    
    /** @brief Set bitrate */
    void setBitrate(double b) {
        bitrate = b;
    }

    /** @brief Constructor*/
    Bitrate(double b=1000000) : BBItem(), bitrate(b) {
    };

    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << " Bitrate is " << bitrate;
        return ost.str();
    }
};

#endif
