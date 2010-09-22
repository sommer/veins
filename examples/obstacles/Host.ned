package org.mixim.examples.obstacles;

import org.mixim.modules.mobility.ConstSpeedMobility;
import org.mixim.base.modules.BaseUtility;

import org.mixim.modules.application.BurstApplLayer;
import org.mixim.base.modules.BaseNetwLayer;
import org.mixim.modules.nic.Nic80211;
import org.mixim.base.modules.BaseArp;	

module Host
{
    parameters:
        @display("p=10,10;b=250,250,rect;o=white");

    gates:
		input radioIn; // gate for sendDirect

    submodules:
        blackboard: BaseUtility {
            @display("p=200,75;b=30,25");
        }

        mobility: ConstSpeedMobility {
            @display("p=200,150;b=30,25");
        }

        appl: BurstApplLayer {
            @display("p=90,50;b=100,20,rect");
        }

        arp: BaseArp {
            @display("p=200,225;b=30,25");
        }

        net: BaseNetwLayer {
            @display("p=90,100;b=100,20,rect");
        }

    	nic: Nic80211 {
    	    @display("p=90,150;b=100,20,rect");
    	}

    connections:
        nic.upperGateOut --> net.lowerGateIn;
        nic.upperGateIn <-- net.lowerGateOut;
    	nic.upperControlOut --> net.lowerControlIn;	
    	nic.upperControlIn <-- net.lowerControlOut;
    
    	net.upperGateOut --> appl.lowerGateIn;
    	net.upperGateIn <-- appl.lowerGateOut;
    	net.upperControlOut --> appl.lowerControlIn;
    	net.upperControlIn <-- appl.lowerControlOut;		
    
    	radioIn --> nic.radioIn;
    
}
