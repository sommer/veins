import TestHost;
import ConnectionManager;
import BaseWorldUtility;
import TestManager;

module Network
{
    parameters:
        double playgroundSizeX; // x size of the area the nodes are in (in meters)
        double playgroundSizeY; // y size of the area the nodes are in (in meters)
        double playgroundSizeZ; // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network
        double run; // total number of hosts in the network
        bool showPassed;

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,10");
    submodules:
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;

        }
        testManager: TestManager;
        channelcontrol: ConnectionManager {
            parameters:
                @display("p=60,30;i=eye14");

        }
        node[numHosts]: TestHost {
            parameters:
                numHosts = numHosts;
                @display("p=50,50;i=laptop");

        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}

network tbpSim extends Network
{
    parameters:
}

