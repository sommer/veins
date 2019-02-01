package org.car2x.veins.base.phyLayer;

// Basic physical layer module.
// See the class "BasePhyLayer" documentation.
simple BasePhyLayer like IWirelessPhy
{
    parameters:
        @class(Veins::BasePhyLayer);

        bool recordStats = default(false); //enable/disable tracking of statistics (eg. cOutvectors)

        bool usePropagationDelay;        //Should transmission delay be simulated?
        double noiseFloor @unit(dBm); // catch-all for all factors negatively impacting SINR (e.g., thermal noise, noise figure, ...)
        bool useNoiseFloor; // should a noise floor be considered when calculating SINR?


        xml antenna = default(xml("<root><Antenna type=\"IsotropicAntenna\" id=\"default_isotropic\"></Antenna></root>"));
        double antennaOffsetX @unit("m") = default(0 m); // Offset of antenna position (x direction) with respect to what a BaseMobility module will tell us (inherited from IChannelAccess)
        double antennaOffsetY @unit("m") = default(0 m); // Offset of antenna position (y direction) with respect to what a BaseMobility module will tell us (inherited from IChannelAccess)
        double antennaOffsetZ @unit("m") = default(0 m); // Offset of antenna position (z direction) with respect to what a BaseMobility module will tell us (inherited from IChannelAccess)
        double antennaOffsetYaw @unit("rad") = default(0 rad); // Offset of antenna orientation (yaw) with respect to what a BaseMobility module will tell us (inherited from IChannelAccess)
        xml analogueModels;             //Specification of the analogue models to use and their parameters
        xml decider;                    //Specification of the decider to use and its parameters

        double minPowerLevel @unit(dBm); // The minimum receive power needed to even attempt decoding a frame

        //# switch times [s]:
        double timeRXToTX       = default(0) @unit(s); // Elapsed time to switch from receive to send state
        double timeRXToSleep    = default(0) @unit(s); // Elapsed time to switch from receive to sleep state

        double timeTXToRX       = default(0) @unit(s); // Elapsed time to switch from send to receive state
        double timeTXToSleep    = default(0) @unit(s); // Elapsed time to switch from send to sleep state

        double timeSleepToRX    = default(0) @unit(s); // Elapsed time to switch from sleep to receive state
        double timeSleepToTX    = default(0) @unit(s); // Elapsed time to switch from sleep to send state

        int initialRadioState   = default(0);   // State the radio is initially in (0=RX, 1=TX, 2=Sleep)

        double radioMinAtt = default(1.0); //radios gain factor (attenuation) while receiving
        double radioMaxAtt = default(0.0); //radios gain factor (attenuation) while not receiving

        int nbRadioChannels = default(1);  // Number of available radio channels. Defaults to single channel radio.
        int initialRadioChannel = default(0);  // Initial radio channel.

    gates:
        input upperLayerIn;     // from the MAC layer
        output upperLayerOut;     // to the MAC layer

        input upperControlIn;     // control from the MAC layer
        output upperControlOut;     // control to the MAC layer

        input radioIn; // for sendDirect from other physical layers

}

