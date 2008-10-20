package org.mixim.modules.netw;

simple BasicSinkRouting
{
    parameters:
        bool debug; // debug switch
        bool stats; // stats switch
        double headerLength; // length of the network packet header (in bits)
        bool autoForward; // automatic forwarding of packets to sink (default: on)
    gates:
        input upperGateIn; // from application layer
        output upperGateOut; // to application layer
        input upperControlIn; // control from application layer
        output upperControlOut; // control to application layer
        input lowerGateIn; // from NIC
        output lowerGateOut; // to NIC
        input lowerControlIn; // control from NIC
        output lowerControlOut; // control to NIC
}

