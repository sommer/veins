<#if wizardType!="simplemodule">
<@setoutput path="${amName}TestNode.ned" />
${bannerComment}

<#if nedPackageName!="">package ${nedPackageName};</#if>

import org.mixim.base.modules.*;

module ${amName}TestNode
{
    parameters:
        string applType; //type of the application layer
        string netwType; //type of the network layer
        string mobType = default("BaseMobility"); //type of the mobility module
        @display("bgb=,,white,,");
    gates:
        input radioIn; // gate for sendDirect
    submodules:
        utility: BaseUtility {
            parameters:
                @display("p=130,38;b=24,24,rect,black,,");
        }
        arp: BaseArp {
            parameters:
                @display("p=130,84;b=24,24,rect,blue,,");
        }
        mobility: <mobType> like IBaseMobility {
            parameters:
                @display("p=130,172;i=cogwheel2");
        }
        appl: <applType> like IBaseApplLayer {
            parameters:
                @display("p=60,50;i=app");
        }
        net: <netwType> like IBaseNetwLayer {
            parameters:
                @display("p=60,108;i=prot1");
        }
        nic: ${amName}TestNic {
            parameters:
                @display("p=60,166;i=iface");
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
</#if>
