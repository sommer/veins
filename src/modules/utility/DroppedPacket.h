/* -*- mode:c++ -*- ********************************************************
 * file:        DroppedPacket.h
 *
 * author:      Andreas Koepke
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
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

#ifndef DroppedPacket_H
#define DroppedPacket_H

#include <sstream>
#include <omnetpp.h>

#include "MiXiMDefs.h"

/**
 * @brief May be published by MAC lyer to indicate why a packet was dropped
 *
 * Reasons: QUEUE: packet dropped because of full queue
 *          CHANNEL: packet dropped because channel could not be aquired
 *          RETRIES: packet dropped because receiver ACKs did not arrive
 *
 *
 * @ingroup utils
 * @author Andreas Koepke
 */
class MIXIM_API  DroppedPacket : public cObject
{
 public:
    /** @brief Possible Reasons */
    enum Reasons
    {
        NONE = 746216,
        QUEUE,
        CHANNEL,
        RETRIES
    };

protected:
    /** @brief Hold Reason why this packet was dropped */
    Reasons reason;

public:

    /** @brief Get Reason */
    Reasons getReason() const {
        return reason;
    }

    /** @brief set the state of the radio*/
    void setReason(Reasons r) {
        reason=r;
    }

    /** @brief Constructor*/
    DroppedPacket(Reasons r=NONE) : cObject(), reason(r) {
    };

    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << " Packet was dropped, because ";
        switch(reason) {
        case NONE:
            ost<<"no reason as yet";
            break;
        case QUEUE:
            ost<<"the queue was full";
            break;
        case CHANNEL:
            ost<<"the transmission channel could not be acquired";
            break;
        case RETRIES:
            ost<<"the numbe of retries was exceeded "
               <<"(the receiver did not respond with an ACK?)";
            break;
        default: ost << "???"; break;
        }
        return ost.str();
    }
};

#endif
