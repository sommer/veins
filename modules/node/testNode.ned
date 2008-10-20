//***************************************************************************
// * file:        testNode.ned
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


package org.mixim.modules.node;

import org.mixim.base.modules.BaseApplLayer;
import org.mixim.base.modules.BaseArp;
import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseNetwLayer;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.nic.MfNic;


module testNode

{
    parameters:
        string applType; //type of the application layer

    //gates:
	    //in: radioIn; // gate for sendDirect

        @display("bgb=250,250,white,,;bgp=10,10");
    submodules:
        //blackboard: Blackboard;
        blackboard: BaseUtility {
            parameters:
                @display("p=200,75;b=30,25");

        }
        mobility: BaseMobility {
            parameters:
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
        nic.upperGateOut --> net.lowerGateIn;
        nic.upperGateIn <-- net.lowerGateOut;
        nic.upperControlOut --> net.lowerControlIn;
        nic.upperControlIn <-- net.lowerControlOut;

        net.upperGateOut --> appl.lowerGateIn;
        net.upperGateIn <-- appl.lowerGateOut;
        net.upperControlOut --> appl.lowerControlIn;
        net.upperControlIn <-- appl.lowerControlOut;

	//radioIn --> nic.radioIn;

}
