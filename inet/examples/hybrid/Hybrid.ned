package mixnet.examples.hybrid;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.StandardHost;

import org.mixim.base.connectionManager.ConnectionManager;

import mixnet.MixnetWorldUtility;



network Hybrid
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        
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


        etherHost: StandardHost {
            @display("r=,,#707070;p=38,132");
            gates:
                ethg[1];

        }
        wlanHost: Ieee80211Host {
            @display("r=,,#707070;p=197,132");
        }

        hybridRouter: HybridRouter {
            @display("r=,,#707070;p=121,99;i=device/accesspoint");
            gates:
                ethg[1];

        }


    connections:
        etherHost.ethg[0] <--> {  delay = 0.1us; } <--> hybridRouter.ethg[0];

}
