simple BaseApplLayer
    parameters:
        debug: bool, // debug switch
        headerLength: numeric const; // length of the application message header (in bits)
    gates:
        in: lowergateIn; // from network layer
        out: lowergateOut; // to network layer
        in: lowerControlIn; // control from network layer
        out: lowerControlOut; // control to network layer
endsimple

