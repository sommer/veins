/* -*- mode:c++ -*- ********************************************************
 * file:        SnrEval80211.h
 *
 * author:      Marc Loebbers
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
 ***************************************************************************/

#ifndef SNR_EVAL_80211H
#define SNR_EVAL_80211H

#include "SnrEval.h"
#include "Bitrate.h"
#include "AirFrame80211_m.h"

/**
 * @brief A SnrEval for the 802.11b protocol
 *
 * Subclass of SnrEval. Basically the same except for some extra
 * parameters of 802.11 and the duration of the packet that has to be
 * computed differently as the modualtion of header and data part of the
 * packet are different. This module forms a physical layer together with
 * the Decider80211 module. The resluting physical layer is intended to
 * be used together with the Mac80211 module.
 *
 * @author Marc Löbbers
 *
 * @ingroup snrEval
 */
class  SnrEval80211 : public SnrEval
{
public:
    Module_Class_Members( SnrEval80211, SnrEval, 0 );

    /** @brief Some extra parameters have to be read in */
    virtual void initialize(int);

    /** @brief Called every time a message arrives -- we need to call a different encaps msg*/
    void handleMessage( cMessage* );

protected:
    AirFrame80211* encapsMsg(cMessage *msg);
    
    /** @brief computes the duration of a 802.11 frame in seconds */
    virtual double calcDuration(cMessage*);

};

#endif
