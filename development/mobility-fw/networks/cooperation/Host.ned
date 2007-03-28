//***************************************************************************
// * file:        Host.ned
// *
// * author:      Marc Loebbers, der Schoenste!
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *              Technische Universitaet Berlin, Germany.
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
// * description: a host in the 802.11 test network
// *              
// ***************************************************************************
// * changelog:   $Revision: 1.4 $
// *              last modified:   $Date: 2007/02/06 18:54:28 $
// *              by:              $Author: tf $
// **************************************************************************/


import
    "StreamApplLayer",
    "SimpleNetwLayer",
    "CooperativeNic",
    "SimpleArp",	
    "CircleMobility",
    "Blackboard";

module Host

    gates:
	in: radioIn; // gate for sendDirect
	out: toSink;

    submodules:
        blackboard: Blackboard;
            display: "p=200,75;b=30,25";

        mobility: CircleMobility;
            display: "p=200,150;b=30,25";

        appl: StreamApplLayer;
            display: "p=90,50;b=100,20,rect";

        arp: SimpleArp;
            display: "p=200,225;b=30,25";

        net: SimpleNetwLayer;
            display: "p=90,100;b=100,20,rect";

	nic: CooperativeNic;
	    display: "p=90,150;b=100,20,rect";

    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
	nic.upperControlOut --> net.lowerControlIn;	
	nic.toSink --> toSink;

	net.uppergateOut --> appl.lowergateIn;
	net.uppergateIn <-- appl.lowergateOut;
	net.upperControlOut --> appl.lowerControlIn;		

	radioIn --> nic.radioIn;
    display: "p=10,10;b=250,250,rect;o=white";
endmodule
