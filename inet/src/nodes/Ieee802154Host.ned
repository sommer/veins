package mixnet.nodes;

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

import org.mixim.modules.nic.Nic802154_TI_CC2420;
import org.mixim.base.modules.IBaseMobility;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;

import mixnet.modules.MixnetBridge;

// This host is an example on how a host with INET network stack
// can be equipped with mobility- and wireless-modules from MiXiM. 
//
// It contains an INET network stack and a MiXiM 802.15.4 NIC (TI_CC2420)
// (connected by the MixnetBridge module), a mobility-module from MiXiM
// as well as a battery module from MiXiM which is used by the NIC.
//
// This host's NIC is able to operate in a multi-ConnectionManager
// network, i.e. more than one physical network in parallel (non-interfering).
module Ieee802154Host
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/wifilaptop;bgb=462,483");
        int numTcpApps = default(0);
        int numUdpApps = default(0);
        string tcpAppType = default("");
        string udpAppType = default("");
        bool IPForward = default(false);
        string routingFile = default("");
        string mobilityType = default("BaseMobility");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
        input radioIn @directIn;
    submodules:
        batteryStats: BatteryStats {
            @display("p=60,106;i=block/circle");
        }
        battery: SimpleBattery {
            @display("p=60,176;i=block/control");
        }
        utility: BaseUtility {
            @display("p=147,41");
        }
        mobility: <mobilityType> like IBaseMobility {
            parameters:
                @display("p=147,128");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,41");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,275");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,346");
        }
        tcpApp[numTcpApps]: <tcpAppType> like TCPApp {
            parameters:
                @display("p=231,67");
        }
        tcp: TCP {
            parameters:
                @display("p=231,154");
        }
        udpApp[numUdpApps]: <udpAppType> like UDPApp {
            parameters:
                @display("p=388,67");
        }
        udp: UDP {
            parameters:
                @display("p=388,154");
        }
        pingApp: PingApp {
            parameters:
                @display("p=231,223");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=316,247;q=queue");
            gates:
                ifIn[(sizeof(pppg)+sizeof(ethg))+1];
                ifOut[(sizeof(pppg)+sizeof(ethg))+1];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=316,408,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=404,427,row,90;q=txQueue");
        }

        bridge: MixnetBridge {
            @display("p=217,332");
        }
        lrwpan: Nic802154_TI_CC2420 {
            parameters:
                @display("p=217,427;q=queue;i=block/ifcard");
        }

    connections allowunconnected:
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        networkLayer.pingOut --> pingApp.pingIn;
        networkLayer.pingIn <-- pingApp.pingOut;

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

        radioIn --> lrwpan.radioIn;
        bridge.upperGateOut --> networkLayer.ifIn[(sizeof(pppg)+sizeof(ethg))];
        bridge.upperGateIn <-- networkLayer.ifOut[(sizeof(pppg)+sizeof(ethg))];

        lrwpan.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> bridge.lowerGateIn;
        lrwpan.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- bridge.lowerGateOut;
        lrwpan.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> bridge.lowerControlIn;
        lrwpan.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- bridge.lowerControlOut;
}

