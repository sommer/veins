package org.mixim.base.phyLayer;

simple BasePhyLayer
{
    parameters:
        bool coreDebug;
        bool usePropagationDelay; 		//Should transmission delay be simulated?
        double thermalNoise;	//the strength of the thermal noise

        xml analogueModels; 			//Specification of the analogue models to use and their parameters
        xml decider;					//Specification of the decider to use and its parameters

        double sensitivity;		//The sensitivity of the physical layer (in dBm)
        double maxTXPower;		//The maximum transimission power of the physical layer

        //switchTimes:
        double timeRXToTX;
        double timeRXToSleep;

        double timeTXToRX;
        double timeTXToSleep;

        double timeSleepToRX;
        double timeSleepToTX;

        int initialRadioState;

    gates:
        input upperGateIn; 	// from the MAC layer
        output upperGateOut; 	// to the MAC layer

        input upperControlIn; 	// control from the MAC layer        
        output upperControlOut; 	// control to the MAC layer

        input radioIn; // for sendDirect from other physical layers

}

