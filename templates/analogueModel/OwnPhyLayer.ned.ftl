<@setoutput path="${phyName}.ned" />
${bannerComment}

<#if nedPackageName!="">package ${nedPackageName};</#if>

import org.mixim.modules.phy.PhyLayer;

simple ${phyName} extends PhyLayer
{
	parameters:
		@class(${phyName});
}