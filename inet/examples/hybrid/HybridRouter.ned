//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

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
import org.mixim.base.modules.IBaseMobility;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.BaseUtility;

import mixnet.MixnetBridge;

module HybridRouter
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        @display("i=device/wifilaptop");
        
        string routingFile = default("");
        string mobilityType = default("BaseMobility");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
        input radioIn @directIn;
    submodules:
    	utility: BaseUtility {
            @display("p=60,388");
        }
        mobility: <mobilityType> like IBaseMobility {
            parameters:
                @display("p=60,445");
        }
        
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "auto";
                routingFile = routingFile;
                @display("p=60,230");
        }
        
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = true;
                @display("p=248,247;q=queue");
            gates:
                ifIn[(sizeof(pppg)+sizeof(ethg))+1];
                ifOut[(sizeof(pppg)+sizeof(ethg))+1];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=205,350,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=240,350,row,90;q=txQueue");
        }
        
        bridge: MixnetBridge {
            @display("p=248,317;i=block/classifier2");
        }
        
        wlan: Nic80211 {
            parameters:
                @display("p=248,388;q=queue");
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

        radioIn --> wlan.radioIn;
        bridge.upperGateOut --> networkLayer.ifIn[(sizeof(pppg)+sizeof(ethg))];
        bridge.upperGateIn <-- networkLayer.ifOut[(sizeof(pppg)+sizeof(ethg))];

        wlan.upperGateOut --> { @display("ls=black;m=m,25,50,25,0"); } --> bridge.lowerGateIn;
        wlan.upperGateIn <-- { @display("ls=black;m=m,15,50,15,0"); } <-- bridge.lowerGateOut;
        wlan.upperControlOut --> { @display("ls=red;m=m,75,50,75,0"); } --> bridge.lowerControlIn;
        wlan.upperControlIn <-- { @display("ls=red;m=m,85,0,85,0"); } <-- bridge.lowerControlOut;
}

