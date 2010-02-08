package org.mixim.examples.mban;

//***************************************************************************
// * file:        MBANHost.ned
// *
// * author:      Jérôme Rousselot
// *
// * copyright:   (C) 2007-2010 CSEM SA, Neuchâtel, Switzerland.
// *
// *              This program is free software; you can redistribute it 
// *              and/or modify it under the terms of the GNU General Public 
// *              License as published by the Free Software Foundation; either
// *              version 2 of the License, or (at your option) any later 
// *              version.
// *              For further information see file COPYING 
// *              in the top level directory
// **************************************************************************/


import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.application.SensorApplLayer;
import org.mixim.modules.netw.DummyRoute;
import org.mixim.modules.netw.ArpHost;
import org.mixim.modules.utility.phyPER;
import org.mixim.modules.nic.INic;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;

module MBANHost
{
    parameters:
        @display("bgb=250,250,white,,;bgp=10,10");
        string nicType;
    gates:
        input radioIn; // gate for sendDirect

    submodules:
        batteryStats: BatteryStats {
            @display("p=239,206;i=block/circle");
        }
        battery: SimpleBattery {
            @display("p=171,206;i=block/control");
        }
        utility: BaseUtility {
            parameters:
                @display("p=130,60;i=block/blackboard");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=130,130;i=block/cogwheel");
        }

        nic: <nicType> like INic {
            parameters:
                @display("p=90,150;b=100,20,rect");
        }
        net: DummyRoute {
            parameters:
        }
        arp: ArpHost {
            @display("p=205,264;b=30,25");
        }
        app: SensorApplLayer {
            parameters:
                @display("p=60,60;i=block/app");

        }
        stats: phyPER { }
                
    connections:
        net.lowerGateOut --> nic.upperGateIn;
        net.lowerGateIn <-- nic.upperGateOut;
        net.lowerControlOut --> nic.upperGateCtrlIn;
        net.lowerControlIn <-- nic.upperGateCtrlOut;

        net.upperGateOut --> app.lowerGateIn;
        net.upperGateIn <-- app.lowerGateOut;
        net.upperControlOut --> app.lowerControlIn;
        net.upperControlIn <-- app.lowerControlOut;


        radioIn --> nic.radioIn;

}

