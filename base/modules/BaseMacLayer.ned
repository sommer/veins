package org.mixim.base.modules;

simple BaseMacLayer
{
    parameters:
        bool coreDebug; // debug switch
        double headerLength; // length of the MAC packet header (in bits)

    gates:
        input upperGateIn; // from net layer
        output upperGateOut; // to net layer
        input upperControlIn; // control from net layer
        output upperControlOut; // control to net layer
        input lowerGateIn; // from phy
        output lowerGateOut; // to phy
        input lowerControlIn; // control from phy
        output lowerControlOut; // control to phy
}

