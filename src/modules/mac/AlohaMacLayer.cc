/* -*- mode:c++ -*- ********************************************************
 * file:        AlohaMacLayer.cc
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this class implements the Aloha MAC protocol for an UWB-IR
 * 				IEEE 802.15.4A transceiver.
 ***************************************************************************/

#include "AlohaMacLayer.h"

#include <iostream>

#include "UWBIRMacPkt_m.h"
#include "UWBIRMacPkt.h"
#include "MacToPhyInterface.h"

using namespace std;

Define_Module(AlohaMacLayer);

void AlohaMacLayer::initialize(int stage) {
	UWBIRMac::initialize(stage);
	if(stage == 1 && myMacAddr != LAddress::L2NULL) {
            phy->setRadioState(Radio::TX);
    } else if(stage == 1 && myMacAddr == LAddress::L2NULL) {
            phy->setRadioState(Radio::RX);
    }
}

void AlohaMacLayer::finish() {
    if(stats) {
        recordScalar("Erroneous bits", errRxBits);
        recordScalar("Total received bits", totalRxBits);
        recordScalar("Average BER", errRxBits/totalRxBits);
        recordScalar("nbReceivedPacketsRS", nbReceivedPacketsRS);
        recordScalar("nbReceivedPacketsnoRS", nbReceivedPacketsNoRS);
        if(rsDecoder) {
        	recordScalar("nbReceivedPackets", nbReceivedPacketsRS);
        } else {
        	recordScalar("nbReceivedPackets", nbReceivedPacketsNoRS);
        }

       	recordScalar("nbSentPackets", nbSentPackets);
    }
}

MacPkt* AlohaMacLayer::encapsMsg(cPacket *msg) {
    UWBIRMacPkt* encaps = new UWBIRMacPkt(msg->getName(), msg->getKind());
    encaps->setByteLength(headerLength);

    // copy dest address from the Control Info attached to the network
    // mesage by the network layer
    cObject *const cInfo = msg->removeControlInfo();

    debugEV <<"CInfo removed, mac addr="<< getUpperDestinationFromControlInfo(cInfo) << endl;
    encaps->setDestAddr(getUpperDestinationFromControlInfo(cInfo));

    //delete the control info
    delete cInfo;

    //set the src address to own mac address
    encaps->setSrcAddr(myMacAddr);

    //encapsulate the network packet
    encaps->encapsulate(msg);

    prepareData(encaps);

    nbSentPackets++;

	return encaps;
}

/*
void AlohaMacLayer::handleUpperMsg(cMessage *msg) {
    MacPkt* packet = encapsMsg(msg);

}
*/
void AlohaMacLayer::handleLowerMsg(cMessage *msg) {
    UWBIRMacPkt *mac = static_cast<UWBIRMacPkt *>(msg);

    if(validatePacket(mac)) {
        const LAddress::L2Type& dest = mac->getDestAddr();
        const LAddress::L2Type& src  = mac->getSrcAddr();
        if ((dest == myMacAddr)) {
        	debugEV << "message with mac addr " << src
                    << " for me (dest=" << dest
                    << ") -> forward packet to upper layer" << std::endl;
            sendUp(decapsMsg(mac));
        } else {
        	debugEV << "message with mac addr " << src
                    << " not for me (dest=" << dest
                    << ") -> delete (my MAC=" << myMacAddr << ")" << std::endl;
            delete mac;
        }
    } else {
    	debugEV << "Errors in message ; dropping mac packet." << std::endl;
        delete mac;
    }
}

