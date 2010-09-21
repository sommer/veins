package mixnet.examples.hybrid;

import org.mixim.modules.nic.Nic802154_TI_CC2420;

// Nic802154_TI_CC2420 extended by the name of the ConnectionManager module
// to enable the use in networks with multiple ConnectionManagers 
module Nic802154_TI_CC2420_MCM extends Nic802154_TI_CC2420
{
    parameters:
        string connectionManagerName;   //name of the ConnectionManager module
}
