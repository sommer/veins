
package org.mixim.examples.CSMAMac;

import org.mixim.modules.nic.WirelessNic;

module PhyMacNic extends WirelessNic
{
    parameters:
        macType = "CSMAMacLayer";
        int id;
}

