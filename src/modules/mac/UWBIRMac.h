/* -*- mode:c++ -*- ********************************************************
 * file:        UWBIRMac.h
 *
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008-2009 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
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
 * description: All MAC designed for use with UWB-IR should derive from this
 * 				class. It provides the necessary functions to build UWBIR
 * 				packets and to receive them.
 ***************************************************************************/
//
// See the following publications for more information:
// [1] An Ultra Wideband Impulse Radio PHY Layer Model for Network Simulation,
// J. Rousselot, J.-D. Decotignie, Simulation: Transactions of the Society
// for Computer Simulation, 2010 (submitted).
// [2] A High-Precision Ultra Wideband Impulse Radio Physical Layer Model
// for Network Simulation, Jérôme Rousselot, Jean-Dominique Decotignie,
// Second International Omnet++ Workshop,Simu'TOOLS, Rome, 6 Mar 09.
// http://portal.acm.org/citation.cfm?id=1537714
//

#ifndef UWBIRMAC_H
#define UWBIRMAC_H

#include "MiXiMDefs.h"
#include "BaseMacLayer.h"
#include "IEEE802154A.h"
#include "Packet.h"

class MacToPhyInterface;
class UWBIRMacPkt;

/**
 * @brief This class provides helper function for MAC modules that use the UWB-IR IEEE 802.15.4A model.
 *
 * Just before sending down a packet to the UWBIRPhyLayer, call prepareData(UWBIRMacPkt* packet).
 * Just after receiving a packet from the UWBIRPhyLayer, call validatePacket(UWBIRMacPkt* packet)
 * and check the returned bool value to know if the packet could be decoded successfully.
 *
 * @ingroup ieee802154a
 * @ingroup macLayer
 */
class MIXIM_API UWBIRMac : public BaseMacLayer {

public:

    virtual void initialize(int stage);

    virtual void finish();

    UWBIRMac(): packet(100) { }

protected:
    bool debug;
    bool stats;
    bool trace;
    bool rsDecoder;
    bool packetsAlwaysValid;
    double totalRxBits, errRxBits; // double and not long as we divide one by the other to get the BER
    MacToPhyInterface* phy;
    Packet packet;
    int prf; // pulse repetition frequency
    cOutVector packetsBER;
    cOutVector dataLengths;
    cOutVector erroneousSymbols;
    cOutVector sentPulses;
    cOutVector receivedPulses;
    cOutVector meanPacketBER;
    cOutVector packetSuccessRate;
    cOutVector packetSuccessRateNoRS;
    cOutVector ber;
    cStdDev meanBER;
    cOutVector RSErrorRate;
    cOutVector success, successNoRS;

    long nbReceivedPacketsNoRS, nbReceivedPacketsRS;
    long nbSentPackets;
    long nbSymbolErrors, nbSymbolsReceived;
    long nbHandledRxPackets;
    long nbFramesDropped;

    // warning: cfg value is currently ignored
    void prepareData(UWBIRMacPkt* packet, IEEE802154A::config cfg = IEEE802154A::cfg_mandatory_4M);

    void handleLowerMsg(cPacket *msg);

    bool validatePacket(UWBIRMacPkt * mac);

    void initCounters();

};

#endif // UWBIRMAC_H

