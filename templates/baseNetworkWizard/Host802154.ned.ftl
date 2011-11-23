<#if protocolName=="CSMA 802.15.4">
<#if nedPackageName!="">package ${nedPackageName};</#if>

import org.mixim.base.modules.*;
import org.mixim.modules.netw.ArpHost;
import org.mixim.modules.transport.Aggregation;
import org.mixim.modules.utility.phyPER;
import org.mixim.modules.nic.Nic802154_TI_CC2420;
import org.mixim.modules.power.battery.SimpleBattery;
import org.mixim.modules.power.battery.BatteryStats;

module Host802154
{
    parameters:
        string netwType; //type of the network layer
        string applType; //type of application layer
        string mobType; //type of mobility module

        @display("bgb=356,318,white;bgp=10,10");
    gates:
        input radioIn;

    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=292,60;i=block/blackboard");
        }
        batteryStats: BatteryStats {
            @display("p=292,257;i=block/circle");
        }
        battery: SimpleBattery {
            @display("p=292,186;i=block/control");
        }
        mobility: <mobType> like IBaseMobility {
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
        transport: Aggregation {
        }
        arp: ArpHost {
            parameters:
                @display("p=202,186");
        }
        appl: <applType> like IBaseApplLayer {
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

        transport.upperGateOut --> appl.lowerGateIn;
        transport.upperGateIn <-- appl.lowerGateOut;
        transport.upperControlOut --> appl.lowerControlIn;
        transport.upperControlIn <-- appl.lowerControlOut;


        radioIn --> nic.radioIn;



}
</#if>
