simple TUMacLayer
    parameters:
        debug: numeric const, // debug switch
        headerLength: numeric const, // length of the MAC packet header (in bits)
		lpl: bool,
		//lpl_on: numeric,
		//lpl_off: numeric,
		siftNodes: numeric;
    gates:
        in: uppergateIn; // from the network layer
        out: uppergateOut; // to the network layer
        in: upperControlIn; // control from the network layer
        out: upperControlOut; // control to the network layer
        in: lowergateIn; // from the physical layer
        out: lowergateOut; // to the physical layer
        in: lowerControlIn; // control from the physical layer
        out: lowerControlOut; // control to the physical layer
endsimple
