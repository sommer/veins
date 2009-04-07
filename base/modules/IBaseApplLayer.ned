package org.mixim.base.modules;

moduleinterface IBaseApplLayer
{
    parameters:
        bool debug; // debug switch
        int headerLength @unit("bit"); // length of the application message header (in bits)
    gates:
        input lowerGateIn; // from network layer
        output lowerGateOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer
}

