//***************************************************************************
// file:        ProbabilistcBroadcast.ned
//
// author:      Damien Piguet, Dimitris Kotsakos, Jérôme Rousselot
// 
// copyright:   (C) 2008 CSEM SA, Neuchâtel, Switzerland.
//
// description: Multi-hop ad-hoc data dissemination protocol base on
//              probabilistic broadcast: depending on application
//              parameters and network status, each packet will
//              go through a number of transmission attempts, each
//              with a certain probability. How number of transmissions
//              attempts and probability of forwarding are computed will
//              be derived from the simulations of this protocol. 
//**************************************************************************

package org.mixim.modules.netw;

// Multi-hop ad-hoc data dissemination protocol based on probabilistic broadcast.
//
// Depending on application
// parameters and network status, each packet will
// go through a number of transmission attempts, each
// with a certain probability. How number of transmissions
// attempts and probability of forwarding are computed will
// be derived from the simulations of this protocol. 
simple AdaptiveProbabilisticBroadcast extends ProbabilisticBroadcast
{
    parameters:
		@class(AdaptiveProbabilisticBroadcast);
        double timeInNeighboursTable @unit(s);
}
