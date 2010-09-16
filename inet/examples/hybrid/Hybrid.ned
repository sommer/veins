package mixnet.examples.hybrid;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.StandardHost;

import org.mixim.base.connectionManager.ConnectionManager;

import mixnet.MixnetWorldUtility;



network Hybrid
{
    parameters:
        //int numHosts;
    
    submodules:
    	world: MixnetWorldUtility {
        }        
        connectionManager: ConnectionManager {
            parameters:
                @display("p=60,50");
        }
        
        
        etherHost: StandardHost {
            @display("r=,,#707070");
            gates:
                ethg[1];
            
        }
        wlanHost: Ieee80211Host {
            @display("r=,,#707070");            
        }
        
        hybridRouter: HybridRouter {
            @display("r=,,#707070");
            gates:
                ethg[1];
            
        }
        
        
    connections:
    	etherHost.ethg[0] <--> {  delay = 0.1us; } <--> hybridRouter.ethg[0];
    	
}
