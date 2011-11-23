package org.mixim.tests.nicTest;

import org.mixim.modules.nic.IWirelessNicUpperCtrl;
import org.mixim.base.modules.BaseMobility;
import org.mixim.modules.netw.ArpHost;

module TestHost
{
    parameters:
        string nicType;
        @display("bgb=180,200,white,,;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        mobility: BaseMobility {
            parameters:
                @display("p=130,130;i=cogwheel2");
        }
        app: TestApp {
            parameters:
                @display("b=32,30;p=60,166");
        }
        nic: <nicType> like IWirelessNicUpperCtrl {
            parameters:
                @display("b=32,30;p=60,166;i=iface");
        }
        arpHost: ArpHost {
            @display("p=36,59");
        }
    connections allowunconnected:
        radioIn --> nic.radioIn;
        app.out --> nic.upperLayerIn;
        app.in <-- nic.upperLayerOut;
        app.controlOut --> nic.upperControlIn;
        app.controlIn <-- nic.upperControlOut;
}
