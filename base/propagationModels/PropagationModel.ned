simple PropagationModel
    parameters:
        debug: numeric const, // debug switch
        sendDirect: bool, // define which ChannelControl module to use
        useTorus: bool, // wrap connections
        playgroundSizeX: numeric const, // x size of the playground (in meters)
        playgroundSizeY: numeric const, // y size of the playground (in meters)
        pMax: numeric const, // maximum sending power used for this network (in mW)
        sat: numeric const, // minimum signal attenuation threshold (in dBm)
        alpha: numeric const, // minimum path loss coefficient
        carrierFrequency: numeric const; // minimum carrier frequency of the channel (in Hz)
endsimple

