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


import BaseApplLayer;
import BaseArp;
import BaseNetwLayer;
import MfNic;
import ConstSpeedMobility;
import BaseMobility;
//"Blackboard",
import BaseUtility;

module Node

{
    parameters:
        string applType; //type of the application layer

        @display("bgb=250,250,white,,;bgp=10,10");
    gates:
        input radioIn; // gate for sendDirect

    submodules:
        utility: BaseUtility {
            parameters:
                @display("b=130,38,rect,black,,;b=24,24");
        //blackboard: Blackboard;
        //blackboard: BaseUtility;
       //     display: "p=200,75;b=30,25";

        }
        mobility: ConstSpeedMobility {
            parameters:
                //mobility: MassMobility;
                @display("p=200,150;b=30,25");

        }
        arp: BaseArp {
            parameters:
                @display("p=200,225;b=30,25");

        }
        appl: <applType> like BaseApplLayer {
            parameters:
                @display("p=90,50;b=100,20,rect");

        }
        net: BaseNetwLayer {
            parameters:
                @display("p=90,100;b=100,20,rect");

        }
        nic: MfNic {
            parameters:
                @display("p=90,150;b=100,20,rect");

        }
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

}
