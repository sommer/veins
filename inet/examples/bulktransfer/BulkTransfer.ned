//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//




package mixnet.examples.bulktransfer;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;

import org.mixim.base.connectionManager.ConnectionManager;
import mixnet.modules.MixnetWorldUtility;
import mixnet.nodes.Ieee80211Host;

//
//This network is based on INET's "inet/bulktransfer" example.
//It is used to show a simple INET network with a host exchanging data over TCP
//using a MiXiM 802.11 network interface card.
//
//IP addresses and default routes are set up automatically, using 
//FlatNetworkConfigurator. Note: FlatNetworkConfigurator does only work with MiXiM
//wireless NICs if default routes for every host are sufficient. This means hosts
//may not have more than one NIC and no routing is necessary.
//
//The network consists of three clients and one server. All three clients exchange
//data with the server.
//The application models bulk file transfer.
//
network BulkTransfer
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
        
        client1: Ieee80211Host {
            parameters:
                @display("p=131,67");
        }
        client2: Ieee80211Host {
            parameters:
                @display("p=337,67");
        }
        client3: Ieee80211Host {
            parameters:
                @display("p=337,247");
        }
        server: Ieee80211Host {
            parameters:
                @display("p=131,247;i=device/pc2");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=62,119");
        }
    connections:
}

