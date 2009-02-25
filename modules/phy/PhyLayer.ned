package org.mixim.modules.phy;

import org.mixim.base.phyLayer.BasePhyLayer;

simple PhyLayer
{
    parameters:
        bool coreDebug;
        bool usePropagationDelay; 		//Should transmission delay be simulated?
        double thermalNoise @unit(dBm);	//the strength of the thermal noise [dBm]
        bool useThermalNoise;			//should thermal noise be considered?

        xml analogueModels; 			//Specification of the analogue models to use and their parameters
        xml decider;					//Specification of the decider to use and its parameters

        double sensitivity @unit(dBm);		//The sensitivity of the physical layer [dBm]
        double maxTXPower @unit(mW);		//The maximum transimission power of the physical layer [mW]

        //switchTimes [s]:
        double timeRXToTX @unit(s);
        double timeRXToSleep @unit(s);

        double timeTXToRX @unit(s);
        double timeTXToSleep @unit(s);

        double timeSleepToRX @unit(s);
        double timeSleepToTX @unit(s);

        int initialRadioState;

    gates:
        input upperGateIn; 	// from the MAC layer
        output upperGateOut; 	// to the MAC layer

        input upperControlIn; 	// control from the MAC layer        
        output upperControlOut; 	// control to the MAC layer

        input radioIn; // for sendDirect from other physical layers
}
