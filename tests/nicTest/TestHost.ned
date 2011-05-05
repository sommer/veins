package org.mixim.tests.nicTest;

import org.mixim.modules.netw.ArpHost;
import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.nic.INic;

module TestHost
{
    parameters:
        string nicType;

        @display("bgb=180,200,white,,;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,60;b=25,25,,black,,");
        }
        mobility: BaseMobility {
            parameters:
                @display("p=130,130;i=cogwheel2");
        }

        app: TestApp {
            parameters:
                @display("b=32,30;p=60,166");
        }

        nic: <nicType> like INic {
            parameters:
                @display("b=32,30;p=60,166;i=iface");
        }
        arpHost: ArpHost {
            @display("p=36,59");
        }
    connections:

        radioIn --> nic.radioIn;


        app.out --> nic.upperGateIn;
        app.controlOut --> nic.upperControlIn;
        app.in <-- nic.upperGateOut;
        app.controlIn <-- nic.upperControlOut;
}

