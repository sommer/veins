<#if nedPackageName!="">package ${nedPackageName};</#if>

<!-->
<#if protocolName=="802.11">
<#assign nicType="Nic80211">
import org.mixim.modules.nic.Nic80211;
<#elseif protocolName=="CSMA using old CSMAMacLayer">
<#assign nicType="CSMANic">
<#else>
<#assign nicType="NicCSMA">
import org.mixim.modules.nic.NicCSMA;
</#if>
</-->
import org.mixim.modules.nic.INic;

import org.mixim.base.modules.*;

module BaseNode
{
    parameters:
        string applType; //type of the application layer
        string netwType; //type of the network layer
        string mobType; //type of the mobility module
        string nicType; //type of the NIC module
        @display("bgb=,,white,,");
    gates:
        input radioIn; // gate for sendDirect
    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,38;b=24,24,rect,black;i=block/blackboard");
        }
        arp: BaseArp {
            parameters:
                @display("p=130,101;b=24,24,rect,blue;i=block/process");
        }
        mobility: <mobType> like IBaseMobility {
            parameters:
                @display("p=130,166;i=block/cogwheel");
        }
        appl: <applType> like IBaseApplLayer {
            parameters:
                @display("p=59,38;i=app");
        }
        net: <netwType> like IBaseNetwLayer {
            parameters:
                @display("p=60,101;i=block/layer");
        }
        nic: <nicType> like INic {
            parameters:
                @display("p=60,166;i=block/ifcard");
        }
    connections:
        nic.upperGateOut --> net.lowerGateIn;
        nic.upperGateIn <-- net.lowerGateOut;
        nic.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> net.lowerControlIn;
        nic.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- net.lowerControlOut;

        net.upperGateOut --> appl.lowerGateIn;
        net.upperGateIn <-- appl.lowerGateOut;
        net.upperControlOut --> { @display("ls=red;m=m,70,0,70,0"); } --> appl.lowerControlIn;
        net.upperControlIn <-- { @display("ls=red;m=m,70,0,70,0"); } <-- appl.lowerControlOut;

        radioIn --> nic.radioIn;

}

