package mixnet.examples.hybrid;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.StandardHost;

import org.mixim.base.connectionManager.ConnectionManager;

import mixnet.MixnetWorldUtility;


// Network containing several hosts equipped with different types of NICs
// as well as a hybrid-router equipped with all of the host's types of NICs
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
                @display("p=159,25");

        }
        wlanChannel: ConnectionManager {
            parameters:
                @display("p=253,25");
        }

        lrwpanChannel: ConnectionManager {
            parameters:
                @display("p=369,25");
        }


        etherHost: StandardHost {
            @display("r=,,#707070;p=38,132");
            gates:
                ethg[1];

        }
        wlanHost: Ieee80211Host {
            @display("r=,,#707070;p=166,181");
        }

        lrwpanHost: Ieee802154Host {
            @display("p=214,86");
        }

        hybridRouter: HybridRouter {
            @display("r=,,#707070;p=121,99;i=device/accesspoint");
            gates:
                ethg[1];

        }


    connections:
        etherHost.ethg[0] <--> {  delay = 0.1us; } <--> hybridRouter.ethg[0];

}
