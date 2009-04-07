package org.mixim.examples.baseNetwork;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;


module BaseNetwork
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numNodes; // total number of hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=0,0");
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


// basic example simulation MiXiM
network baseSim extends BaseNetwork
{
    parameters:
}

