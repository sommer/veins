simple BaseAggLayer
{
    parameters:
        double maxLatency;
        bool debug; // debug switch
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

