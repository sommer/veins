package org.car2x.veins.base.phyLayer;
 
// Wireless physical layer module interface.
moduleinterface IWirelessPhy
{
    parameters:
        bool recordStats; //enable/disable tracking of statistics (eg. cOutvectors)
        int headerLength @unit(bit); //defines the length of the phy header (/preamble)
        
        bool usePropagationDelay;		//Should transmission delay be simulated?
        double thermalNoise @unit(dBm);	//the strength of the thermal noise [dBm]
        bool useThermalNoise;			//should thermal noise be considered?

        xml analogueModels; 			//Specification of the analogue models to use and their parameters
        xml decider;					//Specification of the decider to use and its parameters

		double sensitivity @unit(dBm);	//The sensitivity of the physical layer [dBm]

        //# switch times [s]:
        double timeRXToTX @unit(s); // Elapsed time to switch from receive to send state
        double timeRXToSleep @unit(s); // Elapsed time to switch from receive to sleep state

        double timeTXToRX @unit(s); // Elapsed time to switch from send to receive state
        double timeTXToSleep @unit(s); // Elapsed time to switch from send to sleep state

        double timeSleepToRX @unit(s); // Elapsed time to switch from sleep to receive state
        double timeSleepToTX @unit(s); // Elapsed time to switch from sleep to send state

        int initialRadioState;   // State the radio is initially in (0=RX, 1=TX, 2=Sleep) 
        
        double radioMinAtt; //radios gain factor (attenuation) while receiving
        double radioMaxAtt; //radios gain factor (attenuation) while not receiving

        int nbRadioChannels;  // Number of available radio channels. Defaults to single channel radio.
		int initialRadioChannel;  // Initial radio channel.

    gates:
        input upperLayerIn; 	// from the MAC layer
        output upperLayerOut; 	// to the MAC layer

        input upperControlIn; 	// control from the MAC layer        
        output upperControlOut; 	// control to the MAC layer

        input radioIn; // for sendDirect from other physical layers

}

