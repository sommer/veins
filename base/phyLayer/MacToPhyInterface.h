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
	enum BasePhyMessageKinds {
		/** Indicates the end of a send transmission. */
		TX_OVER = 22000,
		/** Indicates the end of a radio switch. */ 
		RADIO_SWITCHING_OVER,
		/** Channel sense control message between Mac and Phy.*/
		CHANNEL_SENSE_REQUEST,
		/** AirFrame kind */
		AIR_FRAME
	};
	
public:
	
	/**
	 * Returns the current state the radio is in. See RadioState
	 * for possible values.
	 * 
	 * This method is mainly used by the mac layer.
	 */
	virtual int getRadioState() = 0;
	
	/**
	 * Tells the BasePhyLayer to switch to the specified
	 * radio state. The switching process can take some time
	 * depending on the specified switching times in the
	 * ned file. 
	 * The return value is the time needed to switch to the 
	 * specified state, or smaller zero if the radio could
	 * not be switched (propably because it is already switching. 
	 */
	virtual simtime_t setRadioState(int rs) = 0;
	
	/**
	 * Returns the current state of the channel. See ChannelState
	 * for details.
	 */
	virtual ChannelState getChannelState() = 0;

};	

#endif /*MACTOPHYINTERFACE_H_*/
