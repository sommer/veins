package org.mixim.modules.mac;

//
// This class provides helper function for MAC modules that use the UWB-IR IEEE 802.15.4A model.
// Just before sending down a packet to the UWBIRPhyLayer, call prepareData(UWBIRMacPkt* packet).
// Just after receiving a packet from the UWBIRPhyLayer, call validatePacket(UWBIRMacPkt* packet)
// and check the returned bool value to know if the packet could be decoded successfully.
//
simple UWBIRMac

{
    parameters:
        double headerLength @unit(byte);
        bool debug = default(false); // debug switch
        bool stats = default(true);
        bool trace = default(false);
        bool RSDecoder = default(true);
		bool packetsAlwaysValid = default(false);
		
    gates:
        input upperGateIn; // from the network layer
        output upperGateOut; // to the network layer
        input upperControlIn; // control from the network layer
        output upperControlOut; // control to the network layer
        input lowerGateIn; // from the physical layer
        output lowerGateOut; // to the physical layer
        input lowerControlIn; // control from the physical layer
        output lowerControlOut; // control to the physical layer

}

