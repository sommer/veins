//***************************************************************************
// * file:        Host.ned
// *
// * author:      Marc Loebbers
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
// * changelog:   $Revision: 271 $
// *              last modified:   $Date: 2006-05-03 19:13:45 +0200 (Mi, 03 Mai 2006) $
// *              by:              $Author: koepke $
// **************************************************************************/


import
    "BurstApplLayer",
    "SimpleArp",	
    "SimpleNetwLayer",
    "GENic",
    "BasicMobility",
    "Blackboard";

module Host
    gates:
	in: radioIn; // gate for sendDirect

    submodules:
        blackboard: Blackboard;
            display: "p=200,75;b=30,25;o=black";

        mobility: BasicMobility;
            display: "p=200,150;i=cogwheel2";

        arp: SimpleArp;
            display: "p=200,225;b=30,25";

        appl: BurstApplLayer;
            display: "p=90,50;p=60,50;i=app";

        net: SimpleNetwLayer;
            display: "p=90,100;p=60,108;i=prot1";

	nic: GENic;
	    display: "p=90,150;p=60,166;i=iface";



    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
	nic.upperControlOut --> net.lowerControlIn;	

	net.uppergateOut --> appl.lowergateIn;
	net.uppergateIn <-- appl.lowergateOut;
	net.upperControlOut --> appl.lowerControlIn;		

	radioIn --> nic.radioIn;

    display: "p=10,10;b=250,250,rect;o=white";
endmodule
