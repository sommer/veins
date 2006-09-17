//***************************************************************************
//* file:        FloodHost.ned
//*
//* author:      Daniel Willkomm
//*
//* copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
//*              Technische Universitaet Berlin, Germany.
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//***************************************************************************
//* part of:     framework implementation developed by tkn
//* description: FloodHost Compound Module
//***************************************************************************

import
    "BasicApplLayer",
    "Flood",
    "SimpleArp",
    "CoreTestNic",
    "BasicMobility",
    "Blackboard";

// Flood Host
//
// @todo docu
// @author Daniel Willkomm
module FloodHost
    parameters:
        applLayer: string; //application layer type

    gates:
        in: radioIn; //radioIn gate for sendDirect

    submodules:
       blackboard: Blackboard;
            display: "p=130,50;b=25,25;o=black";
        mobility: BasicMobility;
            display: "p=130,150;i=cogwheel2";
        arp: SimpleArp;
            display: "p=130,100;b=25,25";
        appl: applLayer like BasicApplLayer;
            display: "b=35,26;p=60,50;i=app";
        net: Flood;
            display: "b=32,30;p=60,108;i=prot1";
        nic: CoreTestNic;
            display: "b=32,30;p=60,166;i=iface";
    connections:

        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
	nic.upperControlOut --> net.lowerControlIn;	

	net.uppergateOut --> appl.lowergateIn;
	net.uppergateIn <-- appl.lowergateOut;
	net.upperControlOut --> appl.lowerControlIn;		

	radioIn --> nic.radioIn;

    display: "p=10,10;b=180,200,rect;o=white";
endmodule
