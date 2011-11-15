package org.mixim.modules.node;

import org.mixim.base.modules.IMobility;
import org.mixim.modules.nic.IWirelessNicUpperCtrl;
import org.mixim.base.modules.IBaseArp;
import org.mixim.base.modules.IBaseApplLayer;
import org.mixim.base.modules.IBaseNetwLayer;

module WirelessNodeNetwl
{
    parameters:
        string networkType = default("BaseNetwLayer");       //type of the network layer
        string mobilityType = default("ConstSpeedMobility"); //type of the mobility module
        string arpType = default("BaseArp"); //type of address resolution module
        string nicType;
        @display("bgb=210,450,white;i=device/palm;b=40,40,rect");

    gates:
        input radioIn; // gate for sendDirect

    submodules:
        arp: <arpType> like IBaseArp {
                @display("p=140,100;i=block/network2");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=140,30;i=block/cogwheel");
        }
        nic: <nicType> like IWirelessNicUpperCtrl {
            parameters:
                @display("p=70,380;i=block/wrxtx,#008000");
        }
        // OSI network layer
        netwl: <networkType> like IBaseNetwLayer {
            parameters:
                @display("p=70,310;i=block/layer");
        }
    
    connections allowunconnected:
        nic.upperLayerOut --> netwl.lowerLayerIn;
        nic.upperLayerIn <-- netwl.lowerLayerOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> netwl.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- netwl.lowerControlOut;

        radioIn --> nic.radioIn;
}

//
// Standard host module for a wireless MiXiM host with configurable NIC, and OSI // layers, and mobility module.
//
module WirelessNode extends WirelessNodeNetwl
{
    parameters:
        string applicationType = default("BurstApplLayer");  //type of the application layer
        @display("bgb=210,450,white;i=device/palm;b=40,40,rect");

    submodules:
        // OSI transport layer if defined
        appl: <applicationType> like IBaseApplLayer {
            parameters:
                @display("p=70,30;i=app");
        }

    connections:
        netwl.upperLayerOut --> appl.lowerLayerIn;
        netwl.upperLayerIn <-- appl.lowerLayerOut;
        netwl.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        netwl.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;
}

module WirelessNodePlusTran extends WirelessNodeNetwl
{
    parameters:
        string applicationType = default("BurstApplLayer");  //type of the application layer
        string transportType;                                //type of the transport layer
        @display("bgb=210,450,white;i=device/palm;b=40,40,rect");

    submodules:
        // OSI transport layer if defined
        tranl: <transportType> like IBaseNetwLayer {
            parameters:
                @display("p=70,240;i=block/layer");
        }
        appl: <applicationType> like IBaseApplLayer {
            parameters:
                @display("p=70,30;i=app");
        }
    connections:
        //# transport layer exists, so we can connect netwl <--> tranl
        netwl.upperLayerOut --> tranl.lowerLayerIn;
        netwl.upperLayerIn <-- tranl.lowerLayerOut;
        netwl.upperControlOut --> tranl.lowerControlIn;
        netwl.upperControlIn <-- tranl.lowerControlOut;
        
        //# finaly we connect the highest existing layer of transport, session, or presentation to
        //# the application layer if specified
        tranl.upperLayerOut --> appl.lowerLayerIn;
        tranl.upperLayerIn <-- appl.lowerLayerOut;
        tranl.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        tranl.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;
}
