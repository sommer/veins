simple EyesMacLayer
{
    parameters:
        bool debug; // debug switch
        double headerLength; // length of the MAC packet header (in bits)
        bool lpl;
        volatile double siftNodes;
    gates:
        input uppergateIn; // from the network layer
        output uppergateOut; // to the network layer
        input upperControlIn; // control from the network layer
        output upperControlOut; // control to the network layer
        input lowergateIn; // from the physical layer
        output lowergateOut; // to the physical layer
        input lowerControlIn; // control from the physical layer
        output lowerControlOut; // control to the physical layer
}
