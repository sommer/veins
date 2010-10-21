package mixnet.examples.hybrid;

import inet.applications.pingapp.PingApp;
import inet.applications.tcpapp.TCPApp;
import inet.applications.udpapp.UDPApp;
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.nodes.inet.NetworkLayer;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;

import org.mixim.modules.nic.Nic80211;
import org.mixim.modules.nic.Nic802154_TI_CC2420;
import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;

import mixnet.modules.MixnetBridge;

// Router equipped with all types of NICs existent in the network's hosts,
// able to route packages between all of the hosts.
//
// It consists of an INET network-layer, MiXiM NICs (which are each connected
// by a separate MixnetBridge) and INET NICs. 
module HybridRouter
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/wifilaptop;bgb=392,418");

        string routingFile = default("");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
        input wlanRadioIn @directIn;
        input lrwpanRadioIn @directIn;
    submodules:
        batteryStats: BatteryStats {
            @display("p=242,49;i=block/circle");
        }
        battery: SimpleBattery {
            @display("p=314,49;i=block/control");
        }
        utility: BaseUtility {
            @display("p=155,49");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=155,119");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,49");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,131");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "auto";
                routingFile = routingFile;
                @display("p=60,200");
        }

        networkLayer: NetworkLayer {
            parameters:
                proxyARP = true;
                @display("p=265,218;q=queue");
            gates:
                ifIn[(sizeof(pppg)+sizeof(ethg))+2];
                ifOut[(sizeof(pppg)+sizeof(ethg))+2];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=343,370,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=265,345,row,90;q=txQueue");
        }

        wlanBridge: MixnetBridge {
            @display("p=180,280");
        }

        lrwpanBridge: MixnetBridge {
            @display("p=92,280");
        }

        wlan: Nic80211 {
            parameters:
                @display("p=180,370;q=queue");
        }

        lrwpan: Nic802154_TI_CC2420 {
            parameters:
                @display("p=92,370;i=block/ifcard");
        }

    connections allowunconnected:


        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }

        wlanRadioIn --> wlan.radioIn;
        wlanBridge.upperGateOut --> networkLayer.ifIn[(sizeof(pppg)+sizeof(ethg))];
        wlanBridge.upperGateIn <-- networkLayer.ifOut[(sizeof(pppg)+sizeof(ethg))];

        wlan.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> wlanBridge.lowerGateIn;
        wlan.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- wlanBridge.lowerGateOut;
        wlan.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> wlanBridge.lowerControlIn;
        wlan.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- wlanBridge.lowerControlOut;


        lrwpanRadioIn --> lrwpan.radioIn;
        lrwpanBridge.upperGateOut --> networkLayer.ifIn[(sizeof(pppg)+sizeof(ethg))+1];
        lrwpanBridge.upperGateIn <-- networkLayer.ifOut[(sizeof(pppg)+sizeof(ethg))+1];

        lrwpan.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> lrwpanBridge.lowerGateIn;
        lrwpan.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- lrwpanBridge.lowerGateOut;
        lrwpan.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> lrwpanBridge.lowerControlIn;
        lrwpan.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- lrwpanBridge.lowerControlOut;
}

