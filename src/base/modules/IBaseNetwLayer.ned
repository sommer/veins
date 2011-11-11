package org.mixim.base.modules;

// Interface for network layer modules
moduleinterface IBaseNetwLayer
{
    parameters:
        bool stats; // stats switch
        int  headerLength @unit("bit"); // length of the network packet header (in bits)
    gates:
        input upperLayerIn; // from application layer
        output upperLayerOut; // to application layer
        input upperControlIn; // control from application layer
        output upperControlOut; // control to application layer
        input lowerLayerIn; // from NIC
        output lowerLayerOut; // to NIC
        input lowerControlIn; // control from NIC
        output lowerControlOut; // control to NIC
}

