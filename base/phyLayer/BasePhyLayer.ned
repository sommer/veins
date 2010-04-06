package org.mixim.base.phyLayer;

// Basic physical layer module.
// See the class "BasePhyLayer" documentation.
simple BasePhyLayer
{
    parameters:
        bool coreDebug;
        
        bool recordStats = default(false); //enable/disable tracking of statistics (eg. cOutvectors)
        int headerLength = default(0) @unit(bit); //defines the length of the phy header (/preamble)
        
        bool usePropagationDelay;		//Should transmission delay be simulated?
        double thermalNoise @unit(dBm);	//the strength of the thermal noise [dBm]
        bool useThermalNoise;			//should thermal noise be considered?

        xml analogueModels; 			//Specification of the analogue models to use and their parameters
        xml decider;					//Specification of the decider to use and its parameters

		double sensitivity @unit(dBm);	//The sensitivity of the physical layer [dBm]
        double maxTXPower @unit(mW);	//The maximum transimission power of the physical layer [mW]

        //switchTimes [s]:
        double timeRXToTX = default(0) @unit(s);
        double timeRXToSleep = default(0) @unit(s);

        double timeTXToRX = default(0) @unit(s);
        double timeTXToSleep = default(0) @unit(s);

        double timeSleepToRX = default(0) @unit(s);
        double timeSleepToTX = default(0) @unit(s);

        int initialRadioState = default(0); //state the radio is initially in
        
        double radioMinAtt = default(1.0); //radios gain factor (attenuation) while receiving
        double radioMaxAtt = default(0.0); //radios gain factor (attenuation) while not receiving

    gates:
        input upperGateIn; 	// from the MAC layer
        output upperGateOut; 	// to the MAC layer

        input upperControlIn; 	// control from the MAC layer        
        output upperControlOut; 	// control to the MAC layer

        input radioIn; // for sendDirect from other physical layers

}

