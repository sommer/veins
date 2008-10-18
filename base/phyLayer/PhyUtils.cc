#include "PhyUtils.h"

using namespace std;

/**
 * @brief Filters the Signal according to the RadioState (passively),
 * i.e. adding an appropriate instance of RSAMMapping to the Signal
 * 
 * The Signal is added a new RSAMMapping that has a pointer to
 * this instance RadioStateAnalogueModel, hence the pointer is valid as long
 * as the Radio instance exists that has this RSAM as a member.
 */
void RadioStateAnalogueModel::filterSignal(Signal& s)
{
	simtime_t start = s.getSignalStart();
	simtime_t end = start + s.getSignalLength();
	
	RSAMMapping* attMapping = new RSAMMapping(this, start, end);
	s.addAttenuation(attMapping);
}

