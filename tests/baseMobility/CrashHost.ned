package org.mixim.tests.baseMobility;

import org.mixim.base.modules.BaseUtility;


module CrashHost

{
    parameters:
        double numHosts; 		// total number of hosts in the network
        double borderPolicy; 	// this hosts border policy


    submodules:
        blackboard: BaseUtility {
            parameters:
                @display("p=130,60;b=25,25,,black,,");
        }
        mobility: TestBMBaseMobility {
            parameters:
                @display("p=130,130;i=cogwheel2");

        }
}

