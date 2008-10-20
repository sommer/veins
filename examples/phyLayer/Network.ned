package org.mixim.examples.phyLayer;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;


module Network
{
    parameters:
        double playgroundSizeX; // x size of the area the nodes are in (in meters)
        double playgroundSizeY; // y size of the area the nodes are in (in meters)
        double playgroundSizeZ; // z size of the area the nodes are in (in meters)
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
        hybridNode: HybridHost {
            parameters:
                numHosts = 1;
                @display("p=100,50;b=42,42,rect,yellow,,;i=device/antennatower");

        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}

network phySim extends Network
{
    parameters:
}

