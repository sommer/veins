//***************************************************************************
// * file:        Node.ned
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
// * description: a host for the aloha test network
// **************************************************************************/


import
    "BaseApplLayer",
    "BaseArp",
    "BaseNetwLayer",
    "MfNic",
//    "BaseMobility",
    "ConstSpeedMobility",
  //    "Blackboard";
    "BaseUtility";

module Node

    parameters:
        applType: string, //type of the application layer
	id: numeric, // node identifier (temporary: until we get a node id in mixim basenode)
	xEst: numeric, // estimated node position (x coordinate)
	yEst: numeric, // estimated node position (y coordinate)
	zEst: numeric, // estimated node position (z coordinate)
	ts: numeric; // timestamp at which the position estimation was produced

//gates:
//in: radioIn; // gate for sendDirect

    submodules:
//blackboard: Blackboard;
        utility: BaseUtility;
            display: "p=200,75;b=30,25";

        mobility: ConstSpeedMobility; //BaseMobility;
            display: "p=200,150;b=30,25";

        arp: BaseArp;
            display: "p=200,225;b=30,25";

        appl: applType like BaseApplLayer;
            display: "p=90,50;b=100,20,rect";

        net: BaseNetwLayer;
            display: "p=90,100;b=100,20,rect";

	nic: MfNic;
	    display: "p=90,150;b=100,20,rect";

    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
	nic.upperControlOut --> net.lowerControlIn;	
	nic.upperControlIn <-- net.lowerControlOut;	

	net.uppergateOut --> appl.lowergateIn;
	net.uppergateIn <-- appl.lowergateOut;
	net.upperControlOut --> appl.lowerControlIn;		
	net.upperControlIn <-- appl.lowerControlOut;		

//radioIn --> nic.radioIn;

    display: "p=10,10;b=250,250,rect;o=white";
endmodule
