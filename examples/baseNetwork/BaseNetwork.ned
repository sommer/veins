package org.mixim.examples.baseNetwork;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;

//
//This example shows the basic structure of an network using MiXiM.
//
//The network consists of three hosts which use the CSMAMacLayer as MAC layer,
//"SimplePathlossModel" as only AnalogueModel and a SNRThresholdDecider as Decider.
//
//The used Network- and Application layer can be defined in the omnetpp.ini. As default
//the BaseNetwLayer and BurstApplLayer are used.
//
//You can use this network as an template for you own simulation.
//
network BaseNetwork
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numNodes; // total number of hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white;bgp=0,0");
    submodules:
        connectionManager: ConnectionManager {
            parameters:
                @display("p=150,0;b=42,42,rect,green,,;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=30,0;i=misc/globe");
        }
        node[numNodes]: BaseNode {
            parameters:
                @display("p=170,50;i=device/wifilaptop");
        }
    connections allowunconnected:

}

