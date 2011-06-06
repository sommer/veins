
<@setoutput path=targetMainFile />
${bannerComment}

<#if nedPackageName!="">package ${nedPackageName};</#if>

<#if protocolName=="802.11">
<#assign hostType="Host80211">
import org.mixim.modules.node.${hostType};
<#elseif protocolName=="B-MAC">
<#assign hostType="HostBMAC">
import org.mixim.modules.node.${hostType};
<#elseif protocolName=="L-MAC">
<#assign hostType="HostLMAC">
import org.mixim.modules.node.${hostType};
<#elseif protocolName=="CSMA 802.15.4">
<#assign hostType="Host802154">
<#else>
<#assign hostType="HostBasic">
import org.mixim.modules.node.${hostType};
</#if>



import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;

network ${targetTypeName}
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numNodes; // total number of hosts in the network

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white;bgp=0,0");
    submodules:
        connectionManager: ConnectionManager {
            parameters:
                @display("p=150,0;b=42,42,rect,green,,;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=30,0;i=misc/globe");
        }
		node[numNodes]: ${hostType} {}
    connections allowunconnected:

}

