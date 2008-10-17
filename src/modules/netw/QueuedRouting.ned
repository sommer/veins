simple QueuedRouting
{
    parameters:
        bool debug; // debug switch
        bool stats; // stats switch
        double headerLength; // length of the network packet header (in bits)
    gates:
        input uppergateIn; // from application layer
        output uppergateOut; // to application layer
        input upperControlIn; // control from application layer
        output upperControlOut; // control to application layer
        input lowergateIn; // from NIC
        output lowergateOut; // to NIC
        input lowerControlIn; // control from NIC
        output lowerControlOut; // control to NIC
}

