<@setoutput path="${amName}.h" />
${bannerComment}

#ifndef ${(amName?upper_case)}_H_
#define ${(amName?upper_case)}_H_

#include <AnalogueModel.h>

class ${amName} : public AnalogueModel {
protected:
	<#if par1Name!="">${par1CType} ${par1Name};</#if>
	<#if par2Name!="">${par2CType} ${par2Name};</#if>

public:
	${amName}(<#if par1Name!="">${par1CType} ${par1Name}</#if><#if par2Name!="">,${par2CType} ${par2Name}</#if>);
	virtual ~${amName}();

	virtual void filterSignal(Signal& s);
};

#endif /* ${(amName?upper_case)}_H_ */
