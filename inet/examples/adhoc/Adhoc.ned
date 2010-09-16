package mixnet.examples.adhoc;


import inet.networklayer.autorouting.FlatNetworkConfigurator;
import org.mixim.base.connectionManager.ConnectionManager;
import mixnet.MixnetWorldUtility;

network Adhoc
{
    parameters:
        int numHosts;
    submodules:
        world: MixnetWorldUtility {
        }        
        connectionManager: ConnectionManager {
            parameters:
                @display("p=60,50");
        }
        
        host[numHosts]: Ieee80211Host {
            parameters:
                @display("r=,,#707070");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=140,50");
        }
}

