// TODO: remove following comment later
// csma-test-network based on IEEE802.15.4 example

package org.mixim.examples.ieee802154Narrow;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;


module Network
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");
    submodules:
        world: WorldUtilityStats {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=129,40;i=misc/globe");

        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=225,40;b=42,42,rect,yellow;i=abstract/multicast");

        }
        node[numHosts]: PhyMacHost {
            parameters:
                numHosts = numHosts;
                @display("p=75,130;b=42,42,rect,yellow;i=device/wifilaptop");

        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}

network csmaTestSim extends Network
{
    parameters:
}

