package org.mixim.modules.mac;

import org.mixim.base.modules.BaseMacLayer;

//
// This class provides helper function for MAC modules that use the UWB-IR IEEE 802.15.4A model.
// Just before sending down a packet to the UWBIRPhyLayer, call prepareData(UWBIRMacPkt* packet).
// Just after receiving a packet from the UWBIRPhyLayer, call validatePacket(UWBIRMacPkt* packet)
// and check the returned bool value to know if the packet could be decoded successfully.
//
simple UWBIRMac extends BaseMacLayer
{
    parameters:
        @class(UWBIRMac);
        bool debug = default(false); // debug switch
        bool stats = default(true);
        bool trace = default(false);
        bool RSDecoder = default(true);
		bool packetsAlwaysValid = default(false);
		int PRF = default(4);  // Pulse repetition frequency, either 4 or 16 MHz currently
}

