package mixnet.examples.adhoc;


import inet.networklayer.autorouting.FlatNetworkConfigurator;
import org.mixim.base.connectionManager.ConnectionManager;

import mixnet.modules.MixnetWorldUtility;
import mixnet.nodes.Ieee80211Host;

//Network for testing hosts containing an INET network stack,
//a MiXiM NIC (802.11) and a MiXiM mobility module.
network Adhoc
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numHosts; // total number of hosts in the network  

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");

    submodules:
        world: MixnetWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=171,25");

        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=66,25");
        }

        host[numHosts]: Ieee80211Host {
            parameters:
                @display("r=,,#707070");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=249,25;i=block/cogwheel");
        }
}

