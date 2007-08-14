//***************************************************************************
//* file:        BasicHost.ned
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
//* description: BasicHost Compound Module
//*
//***************************************************************************
//* changelog:   $Revision: 327 $
//*              last modified:   $Date: 2006-06-23 21:56:51 +0200 (Fr, 23 Jun 2006) $
//*              by:              $Author: koepke $
//***************************************************************************

import
    "BaseApplLayer",
    "BaseNetwLayer",
    "MfNic",
    "BaseMobility",
    "BaseArp",
    "BaseUtility";



// The BasicHost is a compound module that implements a very
// simple host. It only consists of basic modules and is the
// most basic host you can create for the MF
//
// @author Daniel Willkomm
module BasicHost
    parameters:
        numHosts : numeric const, // total number of hosts in the network
        applLayer: string; //application layer type

    gates:
        in: radioIn;

    submodules:
        blackboard: BaseUtility;
            display: "p=130,60;b=25,25;o=black";
        mobility: BaseMobility;
            display: "p=130,130;i=cogwheel2";
        appl: applLayer like BaseApplLayer;
            display: "b=35,26;p=60,50;i=app";
        net: BaseNetwLayer;
            display: "b=32,30;p=60,108;i=prot1";
        nic: MfNic;
            display: "b=32,30;p=60,166;i=iface";
		arp: BaseArp;
   	     	display: "p=130,180;b=25,25;o=white";
    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
	nic.upperControlOut --> net.lowerControlIn;	
	nic.upperControlIn <-- net.lowerControlOut;			

        net.uppergateOut --> appl.lowergateIn;
        net.uppergateIn <-- appl.lowergateOut;
	net.upperControlOut --> appl.lowerControlIn;		
	net.upperControlIn <-- appl.lowerControlOut;	

	radioIn --> nic.radioIn;

    display: "p=10,10;b=180,200,rect;o=white";
endmodule

