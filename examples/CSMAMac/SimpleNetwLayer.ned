package org.mixim.examples.CSMAMac;

simple SimpleNetwLayer
{
    parameters:
        double ip;
        bool isSwitch;
        double maxTtl;
        double boredTime;
    gates:

        input lowerGateIn; 	// from the Mac layer
        output lowerGateOut; 	// to the Mac layer

        input lowerControlIn; 	// control from the Mac layer        
        output lowerControlOut; 	// control to the Mac layer

}
