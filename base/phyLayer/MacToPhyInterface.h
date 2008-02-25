#ifndef MACTOPHYINTERFACE_H_
#define MACTOPHYINTERFACE_H_

#include <omnetpp.h>

#include "Signal_.h"
#include "ChannelState.h"
#include "PhyUtils.h"


/**
 * 
 * 
 */
class MacToPhyInterface
{
	
public:
	
	/**
	 * Returns the current state the radio is in. See RadioState
	 * for possible values.
	 * 
	 * This method is mainly used by the mac layer.
	 */
	virtual Radio::RadioState getRadioState() = 0;
	
	/**
	 * Tells the BasePhyLayer to switch to the specified
	 * radio state. The switching process can take some time
	 * depending on the specified switching times in the
	 * ned file. 
	 * The return value is the time needed to switch to the 
	 * specified state, or smaller zero if the radio could
	 * not be switched (propably because it is already switching. 
	 */
	virtual simtime_t setRadioState(Radio::RadioState rs) = 0;
	
	/**
	 * Returns the current state of the channel. See ChannelState
	 * for details.
	 */
	virtual ChannelState getChannelState() = 0;

};	

#endif /*MACTOPHYINTERFACE_H_*/
