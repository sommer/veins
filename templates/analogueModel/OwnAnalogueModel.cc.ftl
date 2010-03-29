<@setoutput path="${amName}.cc" />
${bannerComment}

#include "${amName}.h"

${amName}::${amName}(<#if par1Name!="">${par1CType} ${par1Name}</#if><#if par2Name!="">,${par2CType} ${par2Name}</#if>)<#if par1Name!="">:
	${par1Name}(${par1Name})</#if><#if par2Name!="">,
	${par2Name}(${par2Name})</#if>
{}

${amName}::~${amName}() {}

void ${amName}::filterSignal(Signal& s)
{
	simtime_t start = s.getSignalStart();
	simtime_t end = start + s.getSignalLength();
	ev << "Filtering a signal from " << start << " to " << end << "." << endl;

	//create an appropriate mapping to represent our attenuation
	Mapping* attenuation = MappingUtils::createMapping();

	//define the attenuation (as a gain factor) in the mapping (in this case no attenuation)
	attenuation->setValue(Argument(start), 1);
	//TODO: write your own attenuation to the mapping

	//add attenuation to the signals attenuation list
	//this makes our attenuation affect the signal
	s.addAttenuation(attenuation);
}
