/* -*- mode:c++ -*- ********************************************************
 * file:        AlohaMacLayer.h
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

#ifndef ALOHA_MAC_LAYER_H
#define ALOHA_MAC_LAYER_H

#include "UWBIRMac.h"

using namespace std;

class AlohaMacLayer : public UWBIRMac {

public:

    virtual void initialize(int stage);

    virtual void finish();


protected:

    virtual MacPkt* encapsMsg(cPacket *msg);


    virtual void handleLowerMsg(cMessage *msg);



};

#endif // ALOHA_MAC_LAYER_H

