/* -*- mode:c++ -*- ********************************************************
 * file:        GilbertElliotSnr.cc
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


#include "GilbertElliotSnr.h"


Define_Module(GilbertElliotSnr);

/**
 * All values not present in the ned file will be read from the
 * ChannelControl module or assigned default values.
 */
void GilbertElliotSnr::initialize(int stage)
{
    SnrEval::initialize(stage);

    if (stage == 0) {
        meanGood = par("meanGood");
        meanBad = par("meanBad");
        stateChange = new cMessage("Gilbert-Elliot State Duration", 38);
        state = GOOD;
        scheduleAt(simTime() + exponential(meanGood, 0), stateChange);
        EV << "GE state will change at: " << stateChange->arrivalTime() << endl;
    }
}

/**
 * State change timer fired -- do something about it
 */
void GilbertElliotSnr::handleSelfMsg(cMessage *msg)
{
    if(msg == stateChange) {
        EV << "GilbertElliot state changed!\n";
        if(state == GOOD) {
            state = BAD;
            for(cRecvBuff::iterator it = recvBuff.begin(); it != recvBuff.end(); ++it) {
                it->first->setBitError(true);
            }
            scheduleAt(simTime() + exponential(meanBad, 0), stateChange);
        }
        else {
            state = GOOD;
            scheduleAt(simTime() + exponential(meanGood, 0), stateChange);
        }
    }
    else {
        error("GilbertElliotSnr::handleSelfMsg received unknown self messages!");
    }
}

void GilbertElliotSnr::handleLowerMsgStart(AirFrame* msg) {
    if(state == BAD) msg->setBitError(true);
    SnrEval::handleLowerMsgStart(msg);
}

void GilbertElliotSnr::handleLowerMsgEnd(AirFrame* msg) {
    if(state == BAD) msg->setBitError(true);
    SnrEval::handleLowerMsgEnd(msg);
}

void GilbertElliotSnr::finish() {
    SnrEval::finish();
    if (!stateChange->isScheduled()) delete stateChange;
}
