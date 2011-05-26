//***************************************************************************
// file:        ProbabilisticBroadcast.ned
//
// author:      Damien Piguet
// 
// copyright:   (C) 2008-2010 CSEM SA, Neuchâtel, Switzerland.
//
// description: Multi-hop ad-hoc data dissemination protocol based on
//              probabilistic broadcast: depending on application
//              parameters and network status, each packet will
//              go through a number of transmission attempts, each
//              with a certain probability. How number of transmissions
//              attempts and probability of forwarding are computed will
//              be derived from the simulations of this protocol. 
//**************************************************************************
package org.mixim.modules.netw;

import org.mixim.base.modules.BaseNetwLayer;

// Multi-hop ad-hoc data dissemination protocol based on probabilistic broadcast.
//
// Depending on application parameters and network status, each packet will
// go through a number of transmission attempts, each
// with a certain probability. How number of transmissions
// attempts and probability of forwarding are computed will
// be derived from the simulations of this protocol.
simple ProbabilisticBroadcast extends BaseNetwLayer
{
    parameters:
        @class(ProbabilisticBroadcast);
        // debug and trace switches
        // period between two probabilistic broadcast attempts
        double bcperiod @unit(s);
        // probability of broadcast for each attempt
        double beta;
        // maximal number of broadcast attempts for each packet.
        double maxNbBcast;
        // maximal back-off before first broadcast attempt [seconds].
        // if don't want to use this parameter, simply set it
        // to a value that is greater than bcperiod.
        double maxFirstBcastBackoff;
        // How many seconds the message should be kept in queue after its died.
        // That way the message is known if the node receives one of its
        // copy that isn't dead because of TTL de-synchronization due to
        // MAC backoff, propagation delay and clock drift.
        double timeInQueueAfterDeath;

    gates:
        input uppergateIn; // from application layer
        output uppergateOut; // to application layer
        input lowergateIn; // from MAC layer
        output lowergateOut; // to MAC layer
}
