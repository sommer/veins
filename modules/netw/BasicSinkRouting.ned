simple BasicSinkRouting
    parameters:
        debug: numeric const, // debug switch
        headerLength: numeric const, // length of the network packet header (in bits)
		autoForward: bool; // automatic forwarding of packets to sink (default: on)
    gates:
        in: uppergateIn; // from application layer
        out: uppergateOut; // to application layer
        in: upperControlIn; // control from application layer
        out: upperControlOut; // control to application layer
        in: lowergateIn; // from NIC
        out: lowergateOut; // to NIC
        in: lowerControlIn; // control from NIC
        out: lowerControlOut; // control to NIC
endsimple

