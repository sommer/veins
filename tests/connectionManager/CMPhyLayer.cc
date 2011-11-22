/***************************************************************************
 * file:        BasePhyLayer.cc
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


#include "CMPhyLayer.h"


Define_Module(CMPhyLayer);


void CMPhyLayer::handleMessage(cMessage *msg)
{
	if(msg->isSelfMessage()) {
		handleSelfMsg();
	} else {
		MacPkt* m = static_cast<MacPkt*>(msg);
		if(m->getDestAddr() == myAddr() || LAddress::isL2Broadcast(m->getDestAddr())){
			handleLowerMsg(m->getSrcAddr());
		}
	}
	delete msg;
}
