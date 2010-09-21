package mixnet.examples.hybrid;

import org.mixim.modules.nic.Nic80211;

// Nic80211 extended by the name of the ConnectionManager module
// to enable the use in networks with multiple ConnectionManagers 
module Nic80211_MCM extends Nic80211
{
    parameters:
        string connectionManagerName;   //name of the ConnectionManager module
}
