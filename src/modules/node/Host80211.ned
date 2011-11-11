package org.mixim.modules.node;

//
// Example host module using an 802.11 Network interface card.
//
module Host80211 extends WirelessNode
{
    parameters:
        nicType = "Nic80211";
}
