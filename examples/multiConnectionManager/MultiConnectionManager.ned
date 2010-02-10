package org.mixim.examples.multiConnectionManager;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;

//
//This example shows how to use multiple ConnectionManagers in your simulation. 
//
//The network contains two independent ConnectionManagers: "ISMChannel" and "GSMChannel".
//There also is its own list of hosts for each channel "ISMNode" and "GSMNode".
//Both types of hosts use BurstApplLayer <-> BaseNetwLayer <-> CSMAMacLayer <-> 
//PhyLayer as network stack. Whereas the PhyLayer uses "SimplePathlossModel" 
//as AnalogueModel and "SNRThresholdDecider" as Decider.
//
//The channel a host uses is defined in its NICs "connectionManagerName" parameter.
//
//Since the settings for the SimplePathlossModel differ for the GSM- and ISM-Nodes
//there are two seperate <config>.xml for them (GSMmodels.xml and ISMmodels.xml).
//
//The omnetpp.ini contains the following run configurations:
//
//- Config1 - Three hosts which use the same ConnectionManager. No propagation delay.
//- Config2 - Same as Config1 but with propagation delay.
//- Config3 - Two hosts using the GSMChannel ConnectionManager and two host using
//		  	  the ISMChannel ConnectionManager. No propagation delay.
//- Config4 - Same as Config3 but with propagation delay.
//- General - Same as Config3
//
network MultiConnectionManager
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numGSMHosts; // total number of weak hosts in the network
        double numISMHosts; // total number of strong hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=250,0;i=misc/globe");

        }
        ISMChannel: ConnectionManager {
            parameters:
                @display("p=350,0;b=42,42,rect,green,,;i=abstract/multicast");
        }
        GSMChannel: ConnectionManager {
            parameters:
                @display("p=450,0;b=42,42,rect,red,,;i=abstract/multicast");

        }
        GSMNode[numGSMHosts]: PhyMacHost {
            parameters:
                numHosts = numGSMHosts;
                @display("p=50,50;b=42,42,rect,red,,;i=device/palm");

        }
        ISMNode[numISMHosts]: PhyMacHost {
            parameters:
                numHosts = numISMHosts;
                @display("p=75,50;b=42,42,rect,green,,;i=device/wifilaptop");

        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}

