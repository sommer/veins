simple CollisionsPhy
    parameters:
        debug: numeric const, // debug switch
        bitrate: numeric, // bitrate (in bits/sec)
        headerLength: numeric const; // header length of the AirFrame (in bits)
    gates:
        in: uppergateIn; // from the MAC layer
        in: upperControlIn; // control from the MAC layer
        out: uppergateOut; // to the digital phy sub-module
        out: upperControlOut; // control to the digital phy sub-module
endsimple

