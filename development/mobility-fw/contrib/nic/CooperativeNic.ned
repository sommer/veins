//***************************************************************************
// * file:        Nic80211a.ned
// *
// * author:      Thomas Freitag, original Mark Löbbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
// *              (C) 2005 Computer Networks Group, Uni Paderborn
// *
// *              This program is free software; you can redistribute it
// *              and/or modify it under the terms of the GNU General Public
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later
// *              version.
// *              For further information see file COPYING
// *              in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// **************************************************************************/

import
    "Mac80211a",
    "SingleChannelRadio",
    "Decider80211a",
    "ebdaAssignment",
    "SnrEvalWithFading";

//
// This NIC implements an 802.11 network interface card.
//
// @see Mac80211a, Decider80211a, SnrEval80211a
// @author Marc Loebbers
//
module CooperativeNic
    gates:
        in: uppergateIn; // to upper layers
        out: uppergateOut, // from upper layers
             upperControlOut, // control information 
	     toSink; // to recording module
	in: radioIn; // radioIn gate for sendDirect

    submodules:
        mac: Mac80211a;
            display: "p=96,87;i=block/layer";
        decider: CCDecider;
            display: "p=106,157;i=block/process_s";
        snrEval: SnrEvalWithFading;
            display: "p=96,236;i=block/wrxtx";
	radio: SingleChannelRadio;
	    // display: "p=200,30;b=30,25";
        assign: ebdaAssignment;

    connections:
        radioIn --> snrEval.radioIn;
        snrEval.uppergateIn <-- mac.lowergateOut display "m=m,25,0,25,0";
        snrEval.uppergateOut --> decider.lowergateIn display "m=m,73,0,50,50";
        snrEval.upperControlOut --> decider.lowerControlIn;
        decider.uppergateOut --> mac.lowergateIn display "m=m,50,50,75,0";
        decider.upperControlOut --> mac.lowerControlIn;

        mac.uppergateOut --> uppergateOut;
	mac.upperControlOut --> upperControlOut;
        mac.uppergateIn <-- uppergateIn;
	mac.toAssignment --> assign.fromMac;
	mac.fromAssignment <-- assign.toMac;
	mac.toSink --> toSink;
endmodule

