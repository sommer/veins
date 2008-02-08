#ifndef _CHANNELSTATE_H_
#define _CHANNELSTATE_H_

/**
 * Provides information about the current state of the channel:
 * 
 * idle/busy - is the physical currently receiving something?
 * RSSI - the currently received signal strength indicator.
 */
class ChannelState {
protected:
	
	/** defines if the channel is currently idle */
	bool idle;
	
	/** the current RSSI value of the channel */
	double rssi;
public:
	
	/**
	 * Creates and initializes a new ChannelState with the
	 * passed state.
	 * 
	 * isIdle - defines if the channel is currently idle
	 * rssi - the current RSSI value of the channel
	 */
	ChannelState(bool isIdle = false, double rssi = 0.0) :
		idle(isIdle), rssi(rssi) {}
	
	/**
	 * Returns true if the channel is considered idle, meaning
	 * no currently incoming signals.
	 */
	bool isIdle();
	
	/**
	 * Returns the current RSSI value of the channel.
	 */
	double getRSSI();
};

#endif /*_CHANNELSTATE_H_*/
