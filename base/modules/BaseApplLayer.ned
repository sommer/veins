package org.mixim.base.modules;

moduleinterface BaseApplLayer
{
    parameters:
        bool debug; // debug switch
        int headerLength; // length of the application message header (in bits)
    gates:
        input lowerGateIn; // from network layer
        output lowerGateOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer
}

