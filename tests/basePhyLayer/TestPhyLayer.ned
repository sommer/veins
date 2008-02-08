simple TestPhyLayer
    parameters:
        usePropagationDelay: bool, 		//Should transmission delay be simulated?
        thermalNoise: numeric const,	//the strength of the thermal noise
        
        analogueModels: xml, 			//Specification of the analogue models to use and their parameters
        decider: xml,					//Specification of the decider to use and its parameters
        
        sensitivity: numeric const,		//The sensitivity of the physical layer
        maxTXPower:numeric const,		//The maximum transimission power of the physical layer
        
        //switchTimes:
        timeRXToTX: numeric const,
        timeRXToSleep: numeric const,
        
        timeTXToRX: numeric const,
        timeTXToSleep: numeric const,
        
        timeSleepToRX: numeric const,
        timeSleepToTX: numeric const;        
        
    gates:
    	in: upperGateIn; 	// from the MAC layer
        out: upperGateOut; 	// to the MAC layer
        
        in: upperControlIn; 	// control from the MAC layer        
        out: upperControlOut; 	// control to the MAC layer
        
		in: radioIn; // for sendDirect from other physical layers
		
endsimple

