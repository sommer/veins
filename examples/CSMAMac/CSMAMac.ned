package org.mixim.examples.CSMAMac;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;

//
//This example shows how to use MiXiMs CSMA protocol implementation.
//
//The network consists of a number of hosts which represent some kind of switches
//and a number of hosts which represent clients. At first they build up their
//routing tables by sending, forwarding (only switches), receiving and
//evaluating "Hello World" messages. After that phase the clients start randomly
//"jabbering" with other hosts and the switches only forward the "Jabber"
//messages to their destination.
//
//The phy layer uses only "SimplePathlossModel" as AnalogueModel and 
//"SNRThresholdDecider" as Decider.
//
//The omnetpp.ini contains the following configurations:
//
//- "General" 			- three clients in range of each other, no switch, no 
//						  propagation	delay
//- "WithoutPropDelay" 	- four clients and two switches whereas the switches are
//						  in range of each other and the clients are in range of at
//						  least one switch, no propagation delay
//- "WithPropDelay"		- same but with propagation delay
//- "CollissionMac"		- same configuration but the parameters for the CSMAMacLayer
//						  lead to more collisions
//- "Perftest"			- used with CmdEnv for performance testing (time limited)
//
network CSMAMac
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=100,0;i=misc/globe");

        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=200,0;b=42,42,rect,yellow,,;i=abstract/multicast");

        }
        node[numHosts]: PhyMacHost {
            parameters:
                numHosts = numHosts;
                @display("p=50,50;b=42,42,rect,yellow,,;i=device/wifilaptop");

        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}

