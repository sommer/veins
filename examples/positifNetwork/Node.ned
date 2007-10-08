//***************************************************************************
// * file:	Node.ned
// *
// * author:      Marc Loebbers
// *
// * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
// *	      Technische Universitaet Berlin, Germany.
// *
// *	      This program is free software; you can redistribute it 
// *	      and/or modify it under the terms of the GNU General Public 
// *	      License as published by the Free Software Foundation; either
// *	      version 2 of the License, or (at your option) any later 
// *	      version.
// *	      For further information see file COPYING 
// *	      in the top level directory
// ***************************************************************************
// * part of:     framework implementation developed by tkn
// * description: a host for the aloha test network
// **************************************************************************/

import
    "BaseApplLayer",
    "BaseNetwLayer",
    "PositifLayer",
    "BaseNic",
    "BaseMobility",
    "BaseArp",
    "BaseBattery",
    "BaseUtility";

module LocNode
    parameters:
	netType: string,
	locType: string,
	mobType: string,
        applType: string; //type of the application layer
    gates:
	    in: radioIn; // gate for sendDirect
    submodules:
        utility: BaseUtility;
            display: "p=130,38,rect;b=24,24;o=black";
        arp: BaseArp;
            display: "p=130,84,rect;b=24,24;o=blue";
        battery: BaseBattery;
            display: "p=130,128,rect;b=24,24;o=red";
        mobility: mobType like BaseMobility;
            display: "p=130,172;i=cogwheel2";
        appl: applType like BaseApplLayer;
            display: "p=60,50;i=app";
        net: netType like BaseNetwLayer;
            display: "p=60,108;i=prot1";
	loc: locType like PositifLayer;
	    display: "p=60,137";
        nic: BaseNic;
            display: "p=60,166;i=iface";
    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
        nic.upperControlOut --> net.lowerControlIn display "m=m,70,0,70,0;o=red";
        nic.upperControlIn <-- net.lowerControlOut display "m=m,70,0,70,0;o=red";

        net.uppergateOut --> loc.lowergateIn;
        net.uppergateIn <-- loc.lowergateOut;
        net.upperControlOut --> loc.lowerControlIn display "m=m,70,0,70,0;o=red";
        net.upperControlIn <-- loc.lowerControlOut display "m=m,70,0,70,0;o=red";

        loc.uppergateOut --> appl.lowergateIn;
        loc.uppergateIn <-- appl.lowergateOut;
        loc.upperControlOut --> appl.lowerControlIn display "m=m,70,0,70,0;o=red";
        loc.upperControlIn <-- appl.lowerControlOut display "m=m,70,0,70,0;o=red";

	radioIn --> nic.radioIn;

    display: "b=,,rect;o=white";
endmodule

