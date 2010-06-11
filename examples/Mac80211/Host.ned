package org.mixim.examples.Mac80211;

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
            @display("p=200,75;b=30,25;i=block/blackboard");
        }

        mobility: ConstSpeedMobility {
            @display("p=200,150;b=30,25;i=block/cogwheel");
        }

        appl: BurstApplLayer {
            @display("p=91,75;b=22,20,rect;i=block/app");
        }

        arp: BaseArp {
            @display("p=200,225;b=30,25;i=block/process");
        }

        net: BaseNetwLayer {
            @display("p=91,150;b=25,20,rect;i=block/layer");
        }

        nic: Nic80211 {
            @display("p=91,225;b=22,20,rect;i=block/ifcard");
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
