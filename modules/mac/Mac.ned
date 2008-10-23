package org.mixim.modules.mac;

simple EyesMacLayer {
    parameters:
        bool debug;		// debug switch
        double headerLength;	// length of the MAC packet header (in bits)
        bool lpl;
        volatile double siftNodes;

    gates:
        input upperGateIn;	// from the network layer
        output upperGateOut;	// to the network layer
        input upperControlIn;	// control from the network layer
        output upperControlOut;	// control to the network layer
        input lowerGateIn;	// from the physical layer
        output lowerGateOut;	// to the physical layer
        input lowerControlIn;	// control from the physical layer
        output lowerControlOut;	// control to the physical layer
}

