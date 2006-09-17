//***************************************************************************
//* file:        ClientHost.ned
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
//* description: ClientHost Compound Module
//***************************************************************************

import
    "BasicApplLayer",
    "SimpleNetwLayer",
	"SimpleArp",
    "CoreTestNic",
    "ConstSpeedMobility",
    "Blackboard";



// The ClientHost is a compound module that implements a very simple mobile
// client. It only consists of basic modules and the ClientApplLayer.
//
// @author Daniel Willkomm
module ClientHost
    parameters:
    logName: string, //TKEnv display name
        applLayer: string; //application layer type

    gates:
    in: radioIn; // gate for sendDirect

    submodules:
        blackboard: Blackboard;
            display: "p=130,70;b=25,25;o=black";
        mobility: ConstSpeedMobility;
            display: "p=130,140;i=cogwheel2";
        appl: applLayer like BasicApplLayer;
            display: "b=35,26;p=60,50;i=app";
        arp: SimpleArp;
            display: "p=200,225;b=30,25";
        net: SimpleNetwLayer;
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

