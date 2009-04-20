package org.mixim.examples.ieee802154a;

import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.application.TestApplication;
import org.mixim.modules.netw.DummyRoute;

module IEEE802154APhyMacHost
{
    parameters:
        double numHosts; // total number of hosts in the network

        @display("bgb=180,200,white,,;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,60;i=block/blackboard");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=130,130;i=block/cogwheel");
        }
        nic: IEEE802154APhyMacNic {
            parameters:
                @display("b=32,30;p=60,166;i=block/ifcard");
        }
        net: DummyRoute {
            parameters:
        }
        app: TestApplication {
            parameters:
                @display("p=60,60;i=block/app");

        }
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

