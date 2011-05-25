package org.mixim.modules.node;

import org.mixim.base.modules.BaseMobility;
import org.mixim.base.modules.BaseUtility;
import org.mixim.modules.application.SensorApplLayer;
import org.mixim.modules.netw.DummyRoute;
import org.mixim.base.modules.IBaseNetwLayer;
import org.mixim.modules.netw.ArpHost;
import org.mixim.modules.transport.Aggregation;
import org.mixim.modules.utility.phyPER;
import org.mixim.modules.nic.Nic802154_TI_CC2420;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;

//
// This NED module defines a host using an IEEE 802.15.4 transceiver at 2.4GHz
// for wireless communications. It can be used to simulate wireless sensor
// networks. For instance, you can use it to compare your own MAC protocol to
// the 802.15.4 non beacon enabled MAC protocol, or to evaluate IEEE 802.15.4
// wireless sensor networks. It is also a good starting point to define your
// own Host file.
//
// This model was independently validated on a wireless sensor network testbed.
// For more information, see 
// Accurate Timeliness Simulations for Real-Time Wireless Sensor Networks,
// J. Rousselot, J.-D. Decotignie, M. Aoun, P. van der Stok, R. Serna Oliver,
// G. Fohler. In Proceedings of the 2009 Third UKSim European Symposium on Computer 
// Modeling and Simulation. 
//
// @author Jérôme Rousselot
//
// <a href="http://dx.doi.org/10.1109/EMS.2009.34">http://dx.doi.org/10.1109/EMS.2009.34</a>
//
module Host802154_2400MHz
{
    parameters:
        double numHosts; // total number of hosts in the network
        string netwType = default("DummyRoute"); //type of the network layer

        @display("bgb=356,318,white;bgp=10,10;b=40,40,rect;i=device/usb");
    gates:
        input radioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=292,60;i=block/blackboard");
        }
        batteryStats: BatteryStats {
            @display("p=292,257;i=block/circle");
            detail = default(false);
            timeSeries = default(false);
        }
        battery: SimpleBattery {
            @display("p=292,186;i=block/control");
            nominal = default(1000 mAh);
            capacity = default(1000 mAh);
            voltage = default(3 V);
            resolution = default(60 s);
            publishDelta = default(1);
            publishTime = default(60 s);
            numDevices = default(1);
        }
        mobility: BaseMobility {
            parameters:
                @display("p=292,120;i=block/cogwheel");
        }
        nic: Nic802154_TI_CC2420 {
            parameters:
                @display("b=32,30;p=86,265;i=block/ifcard");
        }
        net: <netwType> like IBaseNetwLayer {
            parameters:
                @display("p=86,159");
        }
        transport: Aggregation;
        arp: ArpHost {
            parameters:
                @display("p=202,186");
        }
        app: SensorApplLayer {
            parameters:
                @display("p=86,60;i=block/app");

        }

    connections:
        net.lowerGateOut --> nic.upperGateIn;
        net.lowerGateIn <-- nic.upperGateOut;
        net.lowerControlOut --> nic.upperControlIn;
        net.lowerControlIn <-- nic.upperControlOut;

        net.upperGateOut --> transport.lowerGateIn;
        net.upperGateIn <-- transport.lowerGateOut;
        net.upperControlOut --> transport.lowerControlIn;
        net.upperControlIn <-- transport.lowerControlOut;

        transport.upperGateOut --> app.lowerGateIn;
        transport.upperGateIn <-- app.lowerGateOut;
        transport.upperControlOut --> app.lowerControlIn;
        transport.upperControlIn <-- app.lowerControlOut;


        radioIn --> nic.radioIn;



}

