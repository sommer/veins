package org.mixim.tests.baseMobility;

module CrashHost
{
    parameters:
        double numHosts; 		// total number of hosts in the network
        double borderPolicy; 	// this hosts border policy

    submodules:
        mobility: TestBMBaseMobility {
            parameters:
                @display("p=130,130;i=cogwheel2");

        }
}

