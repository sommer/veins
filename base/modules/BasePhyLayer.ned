simple BasePhyLayer
    parameters:
        debug: numeric const, // debug switch
        transmitterPower: numeric, // power used for transmission of messages (in mW)
        bitrate: numeric, // bitrate (in bits/sec)
        headerLength: numeric const, // header length of the AirFrame (in bits)
        connectionManagerName: string; //name of the connectionmanager
    gates:
        in: uppergateIn; // from the MAC layer
        in: upperControlIn; // control from the MAC layer
        out: uppergateOut; // to the digital phy sub-module
        out: upperControlOut; // control to the digital phy sub-module
	in: radioIn; // for sendDirect
endsimple

