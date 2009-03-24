package org.mixim.modules.mac;

simple UWBIRMac

{
    parameters:
        double headerLength @unit(byte);
        double MACAddr;
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

