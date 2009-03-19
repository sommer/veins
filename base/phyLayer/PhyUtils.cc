#include "PhyUtils.h"

using namespace std;

void RadioStateAnalogueModel::filterSignal(Signal& s)
{
	simtime_t start = s.getSignalStart();
	simtime_t end = start + s.getSignalLength();

	RSAMMapping* attMapping = new RSAMMapping(this, start, end);
	s.addAttenuation(attMapping);
}

