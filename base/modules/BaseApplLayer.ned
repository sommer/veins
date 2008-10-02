simple BaseApplLayer
{
    parameters:
        bool debug; // debug switch
        double headerLength; // length of the application message header (in bits)
    gates:
        input lowergateIn; // from network layer
        output lowergateOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer
}

