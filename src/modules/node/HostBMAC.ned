package org.mixim.modules.node;

import org.mixim.base.modules.IBaseMobility;
import org.mixim.base.modules.IBaseApplLayer;
import org.mixim.base.modules.IBaseNetwLayer;
import org.mixim.modules.netw.ArpHost;
import org.mixim.modules.nic.NicBMAC;
import org.mixim.modules.power.battery.BatteryStats;
import org.mixim.modules.power.battery.SimpleBattery;

//
// Example host module using a BMAC Network interface card.
//
module HostBMAC
{
    parameters:
        string applType; //type of the application layer
        string netwType; //type of the network layer
        string mobType; //type of the mobility module
        @display("bgb=244,294,white;i=device/usb;b=40,40,rect");
    gates:
        input radioIn; // gate for sendDirect
    submodules:
        arp: ArpHost {
            parameters:
                @display("p=186,101;b=24,24,rect,blue;i=block/process");
        }
        mobility: <mobType> like IBaseMobility {
            parameters:
                @display("p=186,166;i=block/cogwheel");
        }
        appl: <applType> like IBaseApplLayer {
            parameters:
                @display("p=59,38;i=app");
        }
        net: <netwType> like IBaseNetwLayer {
            parameters:
                @display("p=60,101;i=block/layer");
        }
        nic: NicBMAC {
            parameters:
                @display("p=60,166;i=block/ifcard");
        }
        batteryStats: BatteryStats {
            @display("p=136,232");
        }
        battery: SimpleBattery {
            @display("p=212,232");
        }
    connections:
        nic.upperLayerOut --> net.lowerLayerIn;
        nic.upperLayerIn <-- net.lowerLayerOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.upperLayerOut --> appl.lowerLayerIn;
        net.upperLayerIn <-- appl.lowerLayerOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}
