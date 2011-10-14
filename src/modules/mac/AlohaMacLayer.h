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
 * description: this class implements a basic Aloha MAC protocol for an UWB-IR
 * 				IEEE 802.15.4A transceiver to allow testing the PHY layer.
 ***************************************************************************/

#ifndef ALOHA_MAC_LAYER_H
#define ALOHA_MAC_LAYER_H

#include "MiXiMDefs.h"
#include "UWBIRMac.h"

/**
 * @brief this class implements a basic Aloha MAC protocol for an UWB-IR
 * IEEE 802.15.4A transceiver to allow testing the PHY layer.
 *
 * This is not a complete implementation of the IEEE 802.15.4A non beacon enabled mode !
 *
 * @ingroup ieee802154a
 * @ingroup macLayer
 */
class MIXIM_API AlohaMacLayer : public UWBIRMac {

public:

    virtual void initialize(int stage);

    virtual void finish();

    int minBE;
    int maxBE;

protected:

    virtual MacPkt* encapsMsg(cPacket *msg);


    virtual void handleLowerMsg(cMessage *msg);

    //virtual void handleUpperMsg(cMessage * msg);

};

#endif // ALOHA_MAC_LAYER_H

