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

import BaseApplLayer;
import BaseNetwLayer;
import PositifLayer;
import BaseNic;
import BaseMobility;
import BaseArp;
import BaseBattery;
import BaseUtility;

module LocNode
{
    parameters:
        string netType;
        string locType;
        string mobType;
        string applType; //type of the application layer
        @display("bgb=,,white,,");
    gates:
        input radioIn; // gate for sendDirect
    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,38,rect;b=24,24,,black,,");
        }
        arp: BaseArp {
            parameters:
                @display("p=130,84,rect;b=24,24,,blue,,");
        }
        battery: BaseBattery {
            parameters:
                @display("p=130,128,rect;b=24,24,,red,,");
        }
        mobility: <mobType> like BaseMobility {
            parameters:
                @display("p=130,172;i=cogwheel2");
        }
        appl: <applType> like BaseApplLayer {
            parameters:
                @display("p=60,50;i=app");
        }
        net: <netType> like BaseNetwLayer {
            parameters:
                @display("p=60,108;i=prot1");
        }
        loc: <locType> like PositifLayer {
            parameters:
                @display("p=60,137");
        }
        nic: BaseNic {
            parameters:
                @display("p=60,166;i=iface");
        }
    connections:
        nic.uppergateOut --> net.lowergateIn;
        nic.uppergateIn <-- net.lowergateOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.uppergateOut --> loc.lowergateIn;
        net.uppergateIn <-- loc.lowergateOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> loc.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- loc.lowerControlOut;

        loc.uppergateOut --> appl.lowergateIn;
        loc.uppergateIn <-- appl.lowergateOut;
        loc.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        loc.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

