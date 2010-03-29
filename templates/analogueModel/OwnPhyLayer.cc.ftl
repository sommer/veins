<@setoutput path="${phyName}.cc" />
${bannerComment}

#include "${phyName}.h"
#include "${amName}.h"

Define_Module(${phyName});

AnalogueModel* ${phyName}::getAnalogueModelFromName(std::string name, ParameterMap& params) {

	if (name == "${amName}")
	{
		return initialize${amName}(params);
	}

	return PhyLayer::getAnalogueModelFromName(name, params);
}

AnalogueModel* ${phyName}::initialize${amName}(ParameterMap& params){
	<#if par1Name!="">
	${par1CType} ${par1Name} = params["${par1Name}"].${par1Type}Value();	
	</#if>
	<#if par2Name!="">
	${par2CType} ${par2Name} = params["${par2Name}"].${par2Type}Value();	
	</#if>

	return new ${amName}(<#if par1Name!="">${par1Name}</#if><#if par2Name!="">, ${par2Name}</#if>);
}
