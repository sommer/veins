package mixnet.examples.hybrid;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.StandardHost;

import org.mixim.base.connectionManager.ConnectionManager;

import mixnet.modules.MixnetWorldUtility;
import mixnet.nodes.Ieee80211Host;
import mixnet.nodes.Ieee802154Host;


//Network for testing a hybrid host (hybridRouter) containing different wired
//(from INET) and wireless (from MiXiM) NICs.
//
//This network contains several hosts, each equipped with a different type of
//NIC, in particular ethernet, 802.11 and 802.15.4 (TI_CC2420).
//
//Further the network contains the hybridRouter which is equipped with
//all of these types of NICs. Thus the hybridRouter is capable of establishing a
//connection to every host in the network as well as route packages between
//hosts.
//
//All hosts which are not the router are equipped with a PING-application. The
//several configurations demonstrate the sending of PINGs between the different
//hosts, routed by the hybridRouter.
//
//Note: NICs connected by different ConnectionManagers (here: 802.11 and
//      802.15.4) cannot interfere with each other yet.
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
