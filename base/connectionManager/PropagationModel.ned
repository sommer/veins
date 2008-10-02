simple PropagationModel
{
    parameters:
        bool debug; // debug switch
        bool sendDirect; // define which ChannelControl module to use
        bool useTorus; // wrap connections
        double playgroundSizeX; // x size of the playground (in meters)
        double playgroundSizeY; // y size of the playground (in meters)
        double pMax; // maximum sending power used for this network (in mW)
        double sat; // minimum signal attenuation threshold (in dBm)
        double alpha; // minimum path loss coefficient
        double carrierFrequency; // minimum carrier frequency of the channel (in Hz)
}

