/* -*- mode:c++ -*- ********************************************************
 * file:        RadioState.h
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

#ifndef RADIOSTATE_H
#define RADIOSTATE_H

#include <omnetpp.h>
#include <Blackboard.h>
#include <sstream>

/**
 * @brief Class to hold the channel state of a certain radio of a nic
 *
 * SLEEP: the radio is sleeping
 * SWITCH_TO_RECV:
 * RECV: the radio is able to receive a message
 * SWITCH_TO_SEND:
 * SEND: the radio is sending a message
 *
 *
 * @ingroup utils
 * @ingroup blackboard
 * @author Andreas Köpke
 * @sa Blackboard
 */
class  RadioState : public BBItem
{
    BBITEM_METAINFO(BBItem);
    
 public:
    /** @brief possible states of this channel of the radio*/
    enum States
    {
        SLEEP,
        RECV,
        SEND,
        SWITCH_TO_SLEEP,
        SWITCH_TO_RECV,
        SWITCH_TO_SEND
    };

protected:
    /** @brief holds the state for this channel */
    States state;

public:

    /** @brief function to get the state*/
    States getState() const {
        return state;
    }
    
    /** @brief set the state of the radio*/
    void setState(States s) {
        state=s;
    }

    /** @brief Constructor*/
    RadioState(States s=RECV) : BBItem(), state(s) {
    };

    /** @brief Enables inspection */
    std::string info() const {
        std::ostringstream ost;
        ost << " RadioState is ";
        switch(state) {
        case SLEEP:
            ost<<"SLEEP";
            break;
        case RECV:
            ost<<"RECV";
            break;
        case SEND:
            ost<<"SEND";
            break;
        case SWITCH_TO_SLEEP:
            ost<<"SWITCH_TO_SLEEP";
            break;
        case SWITCH_TO_RECV:
            ost<<"SWITCH_TO_RECV";
            break;
        case SWITCH_TO_SEND:
            ost<<"SWITCH_TO_SEND";
            break;
        default: ost << "???"; break;
        }
        return ost.str();
    }
};

#endif
