package org.car2x.veins.base.phyLayer;

// Basic physical layer module.
// See the class "BasePhyLayer" documentation.
simple BasePhyLayer like IWirelessPhy
{
    parameters:
        bool coreDebug   = default(false); //debug switch for core framework
        
        bool recordStats = default(false); //enable/disable tracking of statistics (eg. cOutvectors)
        int headerLength = default(0) @unit(bit); //defines the length of the phy header (/preamble)
        
        bool usePropagationDelay;		//Should transmission delay be simulated?
        double thermalNoise @unit(dBm);	//the strength of the thermal noise [dBm]
        bool useThermalNoise;			//should thermal noise be considered?

		
		xml antenna = default(xml("<root><Antenna type=\"IsotropicAntenna\"></Antenna></root>"));
		
		
        xml analogueModels; 			//Specification of the analogue models to use and their parameters
        xml decider;					//Specification of the decider to use and its parameters

		double sensitivity @unit(dBm);	//The sensitivity of the physical layer [dBm]

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
        input upperLayerIn; 	// from the MAC layer
        output upperLayerOut; 	// to the MAC layer

        input upperControlIn; 	// control from the MAC layer        
        output upperControlOut; 	// control to the MAC layer

        input radioIn; // for sendDirect from other physical layers

}

